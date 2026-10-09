#include "Interaction/LRRopeAnchor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Interaction/LRInteractableComponent.h"
#include "Interaction/LRRopeLine.h"
#include "Player/LRPlayerCharacter.h"

ALRRopeAnchor::ALRRopeAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	AnchorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnchorMesh"));
	SetRootComponent(AnchorMesh);

	Interactable = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("Interactable"));
	Interactable->HoldDuration = 2.f;
	Interactable->RequiredToolId = TEXT("Rope");
}

void ALRRopeAnchor::BeginPlay()
{
	Super::BeginPlay();

	if (Interactable)
	{
		Interactable->OnInteractionCompleted.AddDynamic(this, &ALRRopeAnchor::HandleInteractionCompleted);
		if (Interactable->PromptText.IsEmpty())
		{
			Interactable->PromptText = NSLOCTEXT("LastResponse", "SecureRope", "Secure rope");
		}
	}
}

void ALRRopeAnchor::HandleInteractionCompleted(ULRInteractableComponent* InteractableComponent, ALRPlayerCharacter* Interactor)
{
	if (bSecured || !LinkedAnchor || !RopeLineClass || !GetWorld())
	{
		return;
	}

	bSecured = true;

	if (ALRRopeLine* Line = GetWorld()->SpawnActor<ALRRopeLine>(RopeLineClass, FTransform::Identity))
	{
		Line->Initialize(this, LinkedAnchor);
	}

	if (Interactable)
	{
		Interactable->bEnabled = false;
	}

	OnRopeSecured.Broadcast();
	BP_OnRopeSecured();
}
