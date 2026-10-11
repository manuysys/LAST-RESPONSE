#include "Station/LRAlertBoard.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Game/LRGameInstance.h"

ALRAlertBoard::ALRAlertBoard()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	SetRootComponent(BoardMesh);
}

void ALRAlertBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRAlertBoard, CurrentIndex);
}

FContract ALRAlertBoard::GetCurrentContract() const
{
	if (Contracts.IsValidIndex(CurrentIndex))
	{
		return Contracts[CurrentIndex];
	}
	return FContract();
}

void ALRAlertBoard::ServerCycleNext()
{
	if (!HasAuthority() || Contracts.Num() == 0)
	{
		return;
	}

	CurrentIndex = (CurrentIndex + 1) % Contracts.Num();
	OnRep_CurrentIndex();
}

void ALRAlertBoard::ServerAcceptCurrent()
{
	if (!HasAuthority() || !Contracts.IsValidIndex(CurrentIndex))
	{
		return;
	}

	if (ULRGameInstance* GameInstance = Cast<ULRGameInstance>(GetGameInstance()))
	{
		GameInstance->HostMission(Contracts[CurrentIndex]);
	}
}

void ALRAlertBoard::OnRep_CurrentIndex()
{
	OnContractSelected.Broadcast(CurrentIndex);
	BP_OnContractSelected(CurrentIndex);
}
