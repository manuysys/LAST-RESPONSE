#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LRTypes.h"
#include "LRProgressionSubsystem.generated.h"

class ULRStationSave;

UCLASS()
class LASTRESPONSE_API ULRProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetBudget() const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetReputation() const;

	UFUNCTION(BlueprintPure, Category = "Progression")
	float GetStoredVehicleCondition() const;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SetStoredVehicleCondition(float NewCondition);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool TrySpendBudget(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void ApplyMissionResult(const FMissionReport& Report, FName ContractId);

	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsContractCompleted(FName ContractId) const;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SaveStation();

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void LoadStation();

private:
	void EnsureSave();

	UPROPERTY()
	TObjectPtr<ULRStationSave> StationSave;

	static const FString SaveSlotName;
};
