#include "Station/LRAlertBoard.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Game/LRGameInstance.h"
#include "Prototype/LRPrototypeVisuals.h"

ALRAlertBoard::ALRAlertBoard()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	SetRootComponent(BoardMesh);
	BoardMesh->SetRelativeScale3D(FVector(2.0f, 0.2f, 1.2f));
	if (UStaticMesh* Cube = FLRProto::CubeMesh())
	{
		BoardMesh->SetStaticMesh(Cube);
	}
}

void ALRAlertBoard::BeginPlay()
{
	Super::BeginPlay();

	FLRProto::ApplyColor(BoardMesh, FLinearColor(0.12f, 0.14f, 0.22f));
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
