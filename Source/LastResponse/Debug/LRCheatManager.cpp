#include "Debug/LRCheatManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Game/LRMissionDirector.h"
#include "Player/LRPlayerCharacter.h"
#include "Player/LRInventoryComponent.h"
#include "Victim/LRVictimCharacter.h"
#include "World/LRWaterDirector.h"

static ALRMissionDirector* GetMissionDirector(UWorld* World)
{
	for (TActorIterator<ALRMissionDirector> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void ULRCheatManager::LR_SetWater(float NewLevel)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ALRWaterDirector> It(World); It; ++It)
	{
		It->SetWaterLevel(NewLevel);
	}
}

void ULRCheatManager::LR_ForceRise(bool bRise)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ALRWaterDirector> It(World); It; ++It)
	{
		if (bRise)
		{
			It->StartRise();
		}
		else
		{
			It->StopRise();
		}
	}
}

void ULRCheatManager::LR_GiveTool(FName ToolId)
{
	APlayerController* PlayerController = GetOuterAPlayerController();
	ALRPlayerCharacter* Player = PlayerController ? Cast<ALRPlayerCharacter>(PlayerController->GetPawn()) : nullptr;
	ULRInventoryComponent* Inventory = Player ? Player->GetInventoryComponent() : nullptr;

	if (Inventory)
	{
		Inventory->AddTool(ToolId);
	}
}

void ULRCheatManager::LR_WinMission()
{
	if (ALRMissionDirector* Director = GetMissionDirector(GetWorld()))
	{
		Director->EndMission(ELRMissionEndReason::Success);
	}
}

void ULRCheatManager::LR_TeleportToVictim()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = GetOuterAPlayerController();
	ALRPlayerCharacter* Player = PlayerController ? Cast<ALRPlayerCharacter>(PlayerController->GetPawn()) : nullptr;

	if (!World || !Player)
	{
		return;
	}

	for (TActorIterator<ALRVictimCharacter> It(World); It; ++It)
	{
		if (!It->IsSafe())
		{
			Player->SetActorLocation(It->GetActorLocation() + FVector(200.f, 0.f, 50.f));
			return;
		}
	}
}
