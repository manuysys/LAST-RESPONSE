#include "Interaction/LRToolPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Interaction/LRInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"
#include "Player/LRInventoryComponent.h"

ALRToolPickup::ALRToolPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	SetRootComponent(PickupMesh);
	PickupMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.2f));
	if (UStaticMesh* Cube = FLRProto::CubeMesh())
	{
		PickupMesh->SetStaticMesh(Cube);
	}

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

	FString ModelName = TEXT("SM_Tool_PryBar");
	if (ToolId == TEXT("Medkit"))
	{
		ModelName = TEXT("SM_Tool_Medkit");
	}
	else if (ToolId == TEXT("Rope"))
	{
		ModelName = TEXT("SM_Tool_Rope");
	}

	if (!FLRProto::UpgradeToRealModel(PickupMesh, ModelName, FVector::OneVector, FVector(0.f, 0.f, -10.f)))
	{
		FLRProto::ApplyColor(PickupMesh, FLinearColor(0.1f, 0.85f, 0.95f));
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
