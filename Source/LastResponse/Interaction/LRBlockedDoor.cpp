#include "Interaction/LRBlockedDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/LRInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"

ALRBlockedDoor::ALRBlockedDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
	DoorMesh->SetMobility(EComponentMobility::Movable);

	Interactable = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("Interactable"));
	Interactable->HoldDuration = 3.f;
	Interactable->RequiredToolId = TEXT("PryBar");
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
