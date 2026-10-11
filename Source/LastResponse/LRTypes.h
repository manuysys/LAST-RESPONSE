#pragma once

#include "CoreMinimal.h"
#include "LRTypes.generated.h"

UENUM(BlueprintType)
enum class ELRVictimState : uint8
{
	Unlocated		UMETA(DisplayName = "Unlocated"),
	Located			UMETA(DisplayName = "Located"),
	AccessSecured	UMETA(DisplayName = "Access Secured"),
	Assisted		UMETA(DisplayName = "Assisted"),
	Evacuated		UMETA(DisplayName = "Evacuated")
};

UENUM(BlueprintType)
enum class ELRVictimCondition : uint8
{
	Stable		UMETA(DisplayName = "Stable"),
	Injured		UMETA(DisplayName = "Injured"),
	Critical	UMETA(DisplayName = "Critical"),
	Trapped		UMETA(DisplayName = "Trapped")
};

UENUM(BlueprintType)
enum class ELRMissionEndReason : uint8
{
	InProgress,
	Success,
	PlayerIncapacitated,
	TimeExpired
};

USTRUCT(BlueprintType)
struct FMissionReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	ELRMissionEndReason EndReason = ELRMissionEndReason::InProgress;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	float ElapsedSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	int32 VictimsEvacuated = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	int32 VictimsTotal = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	int32 ToolsUsed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	float MaxWaterLevel = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	int32 ObjectivesCompleted = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	int32 ObjectivesTotal = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mission", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VehicleDamage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	int32 BudgetEarned = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	int32 ReputationEarned = 0;
};
