#include "Debug/LRCheatManager.h"
#include "GameFramework/PlayerController.h"
#include "Player/LRPlayerCharacter.h"

namespace
{
	ALRPlayerCharacter* GetLRPlayer(const UCheatManager* CheatManager)
	{
		APlayerController* PlayerController = CheatManager ? CheatManager->GetOuterAPlayerController() : nullptr;
		return PlayerController ? Cast<ALRPlayerCharacter>(PlayerController->GetPawn()) : nullptr;
	}
}

void ULRCheatManager::LR_SetWater(float NewLevel)
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatSetWater(NewLevel);
	}
}

void ULRCheatManager::LR_ForceRise(bool bRise)
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatForceRise(bRise);
	}
}

void ULRCheatManager::LR_GiveTool(FName ToolId)
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatGiveTool(ToolId);
	}
}

void ULRCheatManager::LR_WinMission()
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatWinMission();
	}
}

void ULRCheatManager::LR_TeleportToVictim()
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatTeleportToVictim();
	}
}

void ULRCheatManager::LR_CompleteObjective(FName ObjectiveId)
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatCompleteObjective(ObjectiveId);
	}
}

void ULRCheatManager::LR_SetStorm(float Rain, float Wind)
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatSetStorm(Rain, Wind);
	}
}

void ULRCheatManager::LR_AddBudget(int32 Amount)
{
	if (ALRPlayerCharacter* Player = GetLRPlayer(this))
	{
		Player->ServerCheatAddBudget(Amount);
	}
}
