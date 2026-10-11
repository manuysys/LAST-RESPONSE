#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Meta/LRMetaTypes.h"
#include "LRStationSave.generated.h"

UCLASS()
class LASTRESPONSE_API ULRStationSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Station")
	int32 Budget = 800;

	UPROPERTY(BlueprintReadWrite, Category = "Station")
	int32 Reputation = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Station", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float VehicleCondition = 100.f;

	UPROPERTY(BlueprintReadWrite, Category = "Station")
	TArray<FName> CompletedContracts;
};
