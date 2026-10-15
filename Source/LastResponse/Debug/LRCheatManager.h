#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "LRCheatManager.generated.h"

UCLASS()
class LASTRESPONSE_API ULRCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(exec)
	void LR_SetWater(float NewLevel);

	UFUNCTION(exec)
	void LR_ForceRise(bool bRise);

	UFUNCTION(exec)
	void LR_GiveTool(FName ToolId);

	UFUNCTION(exec)
	void LR_WinMission();

	UFUNCTION(exec)
	void LR_TeleportToVictim();

	UFUNCTION(exec)
	void LR_CompleteObjective(FName ObjectiveId);

	UFUNCTION(exec)
	void LR_SetStorm(float Rain, float Wind);

	UFUNCTION(exec)
	void LR_AddBudget(int32 Amount);
};
