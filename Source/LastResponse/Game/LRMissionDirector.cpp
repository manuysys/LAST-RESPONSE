#include "Game/LRMissionDirector.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "Debug/LRPlaytestRecorder.h"
#include "Meta/LRProgressionSubsystem.h"
#include "Player/LRPlayerCharacter.h"
#include "Vehicle/LRRescueVehicle.h"
#include "Victim/LRVictimCharacter.h"
#include "World/LRWaterDirector.h"

ALRMissionDirector::ALRMissionDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALRMissionDirector::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRMissionDirector, Report);
	DOREPLIFETIME(ALRMissionDirector, bStarted);
}

void ALRMissionDirector::BeginPlay()
{
	Super::BeginPlay();

	if (Victims.Num() == 0)
	{
		for (TActorIterator<ALRVictimCharacter> It(GetWorld()); It; ++It)
		{
			Victims.Add(*It);
		}
	}

	for (const TObjectPtr<ALRVictimCharacter>& Victim : Victims)
	{
		if (Victim)
		{
			Victim->OnVictimStateChanged.AddDynamic(this, &ALRMissionDirector::HandleVictimStateChanged);
		}
	}

	if (!WaterDirector)
	{
		for (TActorIterator<ALRWaterDirector> It(GetWorld()); It; ++It)
		{
			WaterDirector = *It;
			break;
		}
	}

	if (WaterDirector)
	{
		WaterDirector->OnWaterLevelChanged.AddDynamic(this, &ALRMissionDirector::HandleWaterLevelChanged);
	}

	Report.ObjectivesTotal = Objectives.Num();
}

void ALRMissionDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !IsMissionActive())
	{
		return;
	}

	Report.ElapsedSeconds += DeltaSeconds;

	if (TimeLimitSeconds > 0.f && Report.ElapsedSeconds >= TimeLimitSeconds)
	{
		EndMission(ELRMissionEndReason::TimeExpired);
		return;
	}

	CheckTeamWipe();
}

void ALRMissionDirector::StartMission()
{
	if (!HasAuthority() || bStarted)
	{
		return;
	}

	bStarted = true;
	Report = FMissionReport();
	Report.VictimsTotal = Victims.Num();
	Report.ObjectivesTotal = Objectives.Num();

	if (bStartWaterOnMissionStart && WaterDirector)
	{
		WaterDirector->StartRise();
	}

	if (ULRPlaytestRecorder* Recorder = GetWorld()->GetSubsystem<ULRPlaytestRecorder>())
	{
		Recorder->BeginSession(GetWorld()->GetMapName());
	}

	OnMissionStarted.Broadcast();
}

void ALRMissionDirector::EndMission(ELRMissionEndReason Reason)
{
	if (!HasAuthority() || !IsMissionActive())
	{
		return;
	}

	Report.EndReason = Reason;
	Report.VictimsEvacuated = 0;

	for (const TObjectPtr<ALRVictimCharacter>& Victim : Victims)
	{
		if (Victim && Victim->GetVictimState() == ELRVictimState::Evacuated)
		{
			Report.VictimsEvacuated++;
		}
	}

	Report.VehicleDamage = ReadVehicleDamage();
	ComputeRewards();

	if (ULRProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULRProgressionSubsystem>() : nullptr)
	{
		Progression->ApplyMissionResult(Report, ContractId);
	}

	if (ULRPlaytestRecorder* Recorder = GetWorld()->GetSubsystem<ULRPlaytestRecorder>())
	{
		Recorder->EndSession(FString::Printf(TEXT("Reason=%d Evac=%d/%d Budget=%d Rep=%d"),
			static_cast<int32>(Reason), Report.VictimsEvacuated, Report.VictimsTotal,
			Report.BudgetEarned, Report.ReputationEarned));
	}

	OnMissionEnded.Broadcast(Report);
	BP_OnMissionEnded(Report);
}

void ALRMissionDirector::FailMission(ELRMissionEndReason Reason)
{
	EndMission(Reason);
}

void ALRMissionDirector::RegisterToolUse(FName ToolId)
{
	if (!HasAuthority() || !IsMissionActive())
	{
		return;
	}

	Report.ToolsUsed++;

	if (ULRPlaytestRecorder* Recorder = GetWorld()->GetSubsystem<ULRPlaytestRecorder>())
	{
		Recorder->RecordEvent(TEXT("Tool"), ToolId.ToString());
	}
}

void ALRMissionDirector::CompleteObjective(FName ObjectiveId)
{
	if (!HasAuthority() || !IsMissionActive() || ObjectiveId == NAME_None)
	{
		return;
	}

	if (!Objectives.Contains(ObjectiveId) || CompletedObjectives.Contains(ObjectiveId))
	{
		return;
	}

	CompletedObjectives.Add(ObjectiveId);
	Report.ObjectivesCompleted = CompletedObjectives.Num();

	if (ULRPlaytestRecorder* Recorder = GetWorld()->GetSubsystem<ULRPlaytestRecorder>())
	{
		Recorder->RecordEvent(TEXT("Objective"), ObjectiveId.ToString());
	}
}

void ALRMissionDirector::HandleVictimStateChanged(ALRVictimCharacter* Victim, ELRVictimState NewState)
{
	if (ULRPlaytestRecorder* Recorder = GetWorld() ? GetWorld()->GetSubsystem<ULRPlaytestRecorder>() : nullptr)
	{
		Recorder->RecordEvent(TEXT("Victim"), FString::Printf(TEXT("State=%d"), static_cast<int32>(NewState)));
	}

	if (!HasAuthority() || !IsMissionActive() || NewState != ELRVictimState::Evacuated)
	{
		return;
	}

	int32 Evacuated = 0;
	for (const TObjectPtr<ALRVictimCharacter>& Tracked : Victims)
	{
		if (Tracked && Tracked->GetVictimState() == ELRVictimState::Evacuated)
		{
			Evacuated++;
		}
	}

	Report.VictimsEvacuated = Evacuated;

	if (Victims.Num() > 0 && Evacuated >= Victims.Num())
	{
		EndMission(ELRMissionEndReason::Success);
	}
}

void ALRMissionDirector::HandleWaterLevelChanged(float NewLevel)
{
	if (!HasAuthority())
	{
		return;
	}

	Report.MaxWaterLevel = FMath::Max(Report.MaxWaterLevel, NewLevel);
}

void ALRMissionDirector::OnRep_Report()
{
	OnMissionUpdated.Broadcast(Report);
	BP_OnMissionUpdated(Report);

	if (Report.EndReason != ELRMissionEndReason::InProgress && !bEndedNotified)
	{
		bEndedNotified = true;
		OnMissionEnded.Broadcast(Report);
		BP_OnMissionEnded(Report);
	}
}

void ALRMissionDirector::OnRep_Started()
{
}

void ALRMissionDirector::CheckTeamWipe()
{
	int32 PlayerCount = 0;
	int32 DownedCount = 0;

	for (TActorIterator<ALRPlayerCharacter> It(GetWorld()); It; ++It)
	{
		PlayerCount++;
		if (It->IsDowned())
		{
			DownedCount++;
		}
	}

	if (PlayerCount > 0 && DownedCount >= PlayerCount)
	{
		EndMission(ELRMissionEndReason::PlayerIncapacitated);
	}
}

void ALRMissionDirector::ComputeRewards()
{
	const bool bSuccess = Report.EndReason == ELRMissionEndReason::Success;

	Report.BudgetEarned = Report.VictimsEvacuated * 150 + Report.ObjectivesCompleted * 100;
	Report.BudgetEarned -= FMath::RoundToInt(Report.VehicleDamage * 150.f);
	Report.BudgetEarned = FMath::Max(Report.BudgetEarned, 0);
	Report.ReputationEarned = Report.VictimsEvacuated * 10 + Report.ObjectivesCompleted * 5;

	if (bSuccess)
	{
		Report.ReputationEarned += 5;
	}
	else
	{
		Report.BudgetEarned = Report.BudgetEarned / 2;
		Report.ReputationEarned -= 5;
	}
}

float ALRMissionDirector::ReadVehicleDamage() const
{
	for (TActorIterator<ALRRescueVehicle> It(GetWorld()); It; ++It)
	{
		return FMath::Clamp(1.f - It->GetCondition() / 100.f, 0.f, 1.f);
	}

	return 0.f;
}
