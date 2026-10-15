#include "Interaction/LRHeavyDebris.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Interaction/LRCoopInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"

ALRHeavyDebris::ALRHeavyDebris()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DebrisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebrisMesh"));
	SetRootComponent(DebrisMesh);
	DebrisMesh->SetRelativeScale3D(FVector(2.4f, 1.8f, 0.9f));
	if (UStaticMesh* Cube = FLRProto::CubeMesh())
	{
		DebrisMesh->SetStaticMesh(Cube);
	}

	CoopInteractable = CreateDefaultSubobject<ULRCoopInteractableComponent>(TEXT("CoopInteractable"));
	CoopInteractable->HoldDuration = 4.f;
	CoopInteractable->RequiredHolders = 2;
}

void ALRHeavyDebris::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRHeavyDebris, bCleared);
}

void ALRHeavyDebris::BeginPlay()
{
	Super::BeginPlay();

	if (CoopInteractable)
	{
		CoopInteractable->OnInteractionCompleted.AddDynamic(this, &ALRHeavyDebris::HandleInteractionCompleted);
		if (CoopInteractable->PromptText.IsEmpty())
		{
			CoopInteractable->PromptText = NSLOCTEXT("LastResponse", "ClearDebris", "Clear debris (needs 2)");
		}
	}

	if (!FLRProto::UpgradeToRealModel(DebrisMesh, TEXT("SM_Debris"), FVector::OneVector, FVector(0.f, 0.f, -45.f)))
	{
		FLRProto::ApplyColor(DebrisMesh, FLinearColor(0.28f, 0.29f, 0.33f));
	}
}

void ALRHeavyDebris::HandleInteractionCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor)
{
	if (bCleared)
	{
		return;
	}

	bCleared = true;

	OnRep_Cleared();
}

void ALRHeavyDebris::OnRep_Cleared()
{
	if (DebrisMesh)
	{
		DebrisMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DebrisMesh->SetVisibility(false);
	}

	OnDebrisCleared.Broadcast();
	BP_OnDebrisCleared();
}
