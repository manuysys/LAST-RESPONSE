#include "Interaction/LRBlockedDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Interaction/LRInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"

ALRBlockedDoor::ALRBlockedDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
	DoorMesh->SetMobility(EComponentMobility::Movable);
	DoorMesh->SetRelativeScale3D(FVector(1.1f, 0.15f, 2.1f));
	if (UStaticMesh* Cube = FLRProto::CubeMesh())
	{
		DoorMesh->SetStaticMesh(Cube);
	}

	Interactable = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("Interactable"));
	Interactable->HoldDuration = 3.f;
	Interactable->RequiredToolId = TEXT("PryBar");
}

void ALRBlockedDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRBlockedDoor, bOpen);
}

void ALRBlockedDoor::BeginPlay()
{
	Super::BeginPlay();

	if (Interactable)
	{
		Interactable->OnInteractionCompleted.AddDynamic(this, &ALRBlockedDoor::HandleInteractionCompleted);
		if (Interactable->PromptText.IsEmpty())
		{
			Interactable->PromptText = NSLOCTEXT("LastResponse", "ForceDoor", "Force door open");
		}
	}

	if (!FLRProto::UpgradeToRealModel(DoorMesh, TEXT("SM_Door"), FVector::OneVector, FVector::ZeroVector))
	{
		FLRProto::ApplyColor(DoorMesh, FLinearColor(0.42f, 0.26f, 0.12f));
	}
}

void ALRBlockedDoor::HandleInteractionCompleted(ULRInteractableComponent* InteractableComponent, ALRPlayerCharacter* Interactor)
{
	if (bOpen || bAnimating)
	{
		return;
	}

	bAnimating = true;
	bOpen = true;

	if (Interactable)
	{
		Interactable->bEnabled = false;
	}

	OnRep_Open();
}

void ALRBlockedDoor::OnRep_Open()
{
	if (bOpen)
	{
		bAnimating = true;
	}

	OnDoorOpened.Broadcast();
	BP_OnDoorOpened();
}

void ALRBlockedDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bAnimating || !DoorMesh)
	{
		return;
	}

	OpenAlpha = FMath::Clamp(OpenAlpha + DeltaSeconds / FMath::Max(OpenDuration, 0.01f), 0.f, 1.f);
	DoorMesh->SetRelativeRotation(FRotator(0.f, OpenAngle * OpenAlpha, 0.f));

	if (OpenAlpha >= 1.f)
	{
		bAnimating = false;
	}
}
