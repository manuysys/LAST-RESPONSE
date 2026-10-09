#include "Interaction/LRToolPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/LRInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"
#include "Player/LRInventoryComponent.h"

ALRToolPickup::ALRToolPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	SetRootComponent(PickupMesh);

	Interactable = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("Interactable"));
	Interactable->HoldDuration = 0.5f;
}

void ALRToolPickup::BeginPlay()
{
	Super::BeginPlay();

	if (Interactable)
	{
		Interactable->OnInteractionCompleted.AddDynamic(this, &ALRToolPickup::HandleInteractionCompleted);
		if (Interactable->PromptText.IsEmpty())
		{
			Interactable->PromptText = NSLOCTEXT("LastResponse", "PickupTool", "Pick up tool");
		}
	}
}

void ALRToolPickup::HandleInteractionCompleted(ULRInteractableComponent* InteractableComponent, ALRPlayerCharacter* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	if (ULRInventoryComponent* Inventory = Interactor->GetInventoryComponent())
	{
		Inventory->AddTool(ToolId);
	}

	if (Interactable)
	{
		Interactable->bEnabled = false;
	}

	if (bDestroyOnPickup)
	{
		Destroy();
	}
	else
	{
		SetActorHiddenInGame(true);
	}
}
