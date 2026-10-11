#include "Game/LRGameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void ULRGameInstance::SetActiveContract(const FContract& NewContract)
{
	ActiveContract = NewContract;
}

void ULRGameInstance::HostMission(const FContract& Contract)
{
	SetActiveContract(Contract);
	const FString URL = FString::Printf(TEXT("%s?listen"), *Contract.MapName.ToString());

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(URL);
	}
	else
	{
		UGameplayStatics::OpenLevel(this, Contract.MapName, true, TEXT("listen"));
	}
}

void ULRGameInstance::JoinServer(const FString& Address)
{
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		PlayerController->ClientTravel(Address, TRAVEL_Absolute);
	}
}

void ULRGameInstance::ReturnToStation()
{
	ActiveContract = FContract();
	const FString URL = FString::Printf(TEXT("%s?listen"), *StationMapName.ToString());

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(URL);
	}
	else
	{
		UGameplayStatics::OpenLevel(this, StationMapName, true, TEXT("listen"));
	}
}
