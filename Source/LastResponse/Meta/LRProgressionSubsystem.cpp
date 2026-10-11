#include "Meta/LRProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Meta/LRStationSave.h"

const FString ULRProgressionSubsystem::SaveSlotName = TEXT("StationSlot");

void ULRProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadStation();
}

int32 ULRProgressionSubsystem::GetBudget() const
{
	return StationSave ? StationSave->Budget : 0;
}

int32 ULRProgressionSubsystem::GetReputation() const
{
	return StationSave ? StationSave->Reputation : 0;
}

float ULRProgressionSubsystem::GetStoredVehicleCondition() const
{
	return StationSave ? StationSave->VehicleCondition : 100.f;
}

void ULRProgressionSubsystem::SetStoredVehicleCondition(float NewCondition)
{
	EnsureSave();
	StationSave->VehicleCondition = FMath::Clamp(NewCondition, 0.f, 100.f);
	SaveStation();
}

bool ULRProgressionSubsystem::TrySpendBudget(int32 Amount)
{
	EnsureSave();
	if (Amount < 0 || StationSave->Budget < Amount)
	{
		return false;
	}

	StationSave->Budget -= Amount;
	SaveStation();
	return true;
}

void ULRProgressionSubsystem::ApplyMissionResult(const FMissionReport& Report, FName ContractId)
{
	EnsureSave();
	StationSave->Budget += Report.BudgetEarned;
	StationSave->Reputation = FMath::Max(StationSave->Reputation + Report.ReputationEarned, 0);

	if (Report.VehicleDamage > 0.f)
	{
		StationSave->VehicleCondition = FMath::Clamp(StationSave->VehicleCondition - Report.VehicleDamage * 100.f, 0.f, 100.f);
	}

	if (ContractId != NAME_None && Report.EndReason == ELRMissionEndReason::Success)
	{
		StationSave->CompletedContracts.AddUnique(ContractId);
	}

	SaveStation();
}

bool ULRProgressionSubsystem::IsContractCompleted(FName ContractId) const
{
	return StationSave && StationSave->CompletedContracts.Contains(ContractId);
}

void ULRProgressionSubsystem::SaveStation()
{
	EnsureSave();
	UGameplayStatics::SaveGameToSlot(StationSave, SaveSlotName, 0);
}

void ULRProgressionSubsystem::LoadStation()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		StationSave = Cast<ULRStationSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}

	EnsureSave();
}

void ULRProgressionSubsystem::EnsureSave()
{
	if (!StationSave)
	{
		StationSave = Cast<ULRStationSave>(UGameplayStatics::CreateSaveGameObject(ULRStationSave::StaticClass()));
	}
}
