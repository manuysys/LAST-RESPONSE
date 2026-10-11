#pragma once

#include "CoreMinimal.h"
#include "LRMetaTypes.generated.h"

USTRUCT(BlueprintType)
struct FContract
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	FName ContractId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	FName MapName = TEXT("M_FloodTest");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	float WaterLevel = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	float RiseSpeed = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	int32 ExpectedVictims = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	float TimeLimitSeconds = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	int32 RewardBudget = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	int32 RewardReputation = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contract")
	bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FToolEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FName ToolId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 Price = 100;
};
