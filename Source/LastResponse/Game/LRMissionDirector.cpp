#include "Game/LRMissionDirector.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Victim/LRVictimCharacter.h"
#include "World/LRWaterDirector.h"

ALRMissionDirector::ALRMissionDirector()
{
	PrimaryActorTick.bCanEverTick = true;
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
}

void ALRMissionDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsMissionActive())
	{
		return;
	}

	Report.ElapsedSeconds += DeltaSeconds;

	if (TimeLimitSeconds > 0.f && Report.ElapsedSeconds >= TimeLimitSeconds)
	{
		EndMission(ELRMissionEndReason::TimeExpired);
	}
}

void ALRMissionDirector::StartMission()
{
	if (bStarted)
	{
		return;
	}

	bStarted = true;
	Report = FMissionReport();
	Report.VictimsTotal = Victims.Num();

	if (bStartWaterOnMissionStart && WaterDirector)
	{
		WaterDirector->StartRise();
	}

	OnMissionStarted.Broadcast();
}

void ALRMissionDirector::EndMission(ELRMissionEndReason Reason)
{
	if (!IsMissionActive())
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

	OnMissionEnded.Broadcast(Report);
	BP_OnMissionEnded(Report);
}

void ALRMissionDirector::FailMission(ELRMissionEndReason Reason)
{
	EndMission(Reason);
}

void ALRMissionDirector::RegisterToolUse(FName ToolId)
{
	if (!IsMissionActive())
	{
		return;
	}

	Report.ToolsUsed++;
}

void ALRMissionDirector::HandleVictimStateChanged(ALRVictimCharacter* Victim, ELRVictimState NewState)
{
	if (!IsMissionActive() || NewState != ELRVictimState::Evacuated)
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
	Report.MaxWaterLevel = FMath::Max(Report.MaxWaterLevel, NewLevel);
}
