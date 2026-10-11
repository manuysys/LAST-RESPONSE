#include "Station/LRLoadoutLocker.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/LRInteractableComponent.h"
#include "Meta/LRMetaLibrary.h"
#include "Player/LRPlayerCharacter.h"
#include "Player/LRInventoryComponent.h"

ALRLoadoutLocker::ALRLoadoutLocker()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	LockerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockerMesh"));
	SetRootComponent(LockerMesh);

	Interactable = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("Interactable"));
	Interactable->HoldDuration = 1.f;
}

void ALRLoadoutLocker::BeginPlay()
{
	Super::BeginPlay();

	if (Interactable)
	{
		Interactable->OnInteractionCompleted.AddDynamic(this, &ALRLoadoutLocker::HandleInteractionCompleted);
		if (Interactable->PromptText.IsEmpty())
		{
			Interactable->PromptText = NSLOCTEXT("LastResponse", "TakeLoadout", "Take loadout");
		}
	}
}

void ALRLoadoutLocker::HandleInteractionCompleted(ULRInteractableComponent* InteractableComponent, ALRPlayerCharacter* Interactor)
{
	if (!HasAuthority() || !Interactor)
	{
		return;
	}

	float TotalWeight = 0.f;
	for (const FName& ToolId : LoadoutTools)
	{
		TotalWeight += ULRMetaLibrary::GetToolWeight(ToolId);
	}

	if (TotalWeight > MaxLoadoutWeight)
	{
		OnLoadoutRejected.Broadcast(TotalWeight);
		BP_OnLoadoutRejected(TotalWeight);
		return;
	}

	if (ULRInventoryComponent* Inventory = Interactor->GetInventoryComponent())
	{
		Inventory->SetTools(LoadoutTools);
	}

	OnLoadoutApplied.Broadcast();
	BP_OnLoadoutApplied();
}
