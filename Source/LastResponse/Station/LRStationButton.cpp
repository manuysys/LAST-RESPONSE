#include "Station/LRStationButton.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/LRInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Station/LRAlertBoard.h"
#include "Station/LRWorkshop.h"

ALRStationButton::ALRStationButton()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	SetRootComponent(ButtonMesh);
	ButtonMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.15f));
	if (UStaticMesh* Cylinder = FLRProto::CylinderMesh())
	{
		ButtonMesh->SetStaticMesh(Cylinder);
	}

	Interactable = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("Interactable"));
	Interactable->HoldDuration = 0.5f;
}

void ALRStationButton::BeginPlay()
{
	Super::BeginPlay();

	if (!FLRProto::UpgradeToRealModel(ButtonMesh, TEXT("SM_Button"), FVector::OneVector, FVector(0.f, 0.f, -7.5f)))
	{
		FLRProto::ApplyColor(ButtonMesh, FLinearColor(0.9f, 0.12f, 0.1f));
	}

	if (Interactable)
	{
		Interactable->OnInteractionCompleted.AddDynamic(this, &ALRStationButton::HandleInteractionCompleted);
		if (Interactable->PromptText.IsEmpty())
		{
			switch (Action)
			{
			case EStationButtonAction::BoardNext:
				Interactable->PromptText = NSLOCTEXT("LastResponse", "NextContract", "Next contract");
				break;
			case EStationButtonAction::BoardAccept:
				Interactable->HoldDuration = 1.5f;
				Interactable->PromptText = NSLOCTEXT("LastResponse", "AcceptContract", "Accept contract");
				break;
			case EStationButtonAction::WorkshopRepair:
				Interactable->HoldDuration = 1.5f;
				Interactable->PromptText = NSLOCTEXT("LastResponse", "RepairVehicle", "Repair vehicle");
				break;
			default:
				Interactable->PromptText = NSLOCTEXT("LastResponse", "BuySupplies", "Buy supplies");
				break;
			}
		}
	}
}

void ALRStationButton::HandleInteractionCompleted(ULRInteractableComponent* InteractableComponent, ALRPlayerCharacter* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	switch (Action)
	{
	case EStationButtonAction::BoardNext:
		if (ALRAlertBoard* Board = Cast<ALRAlertBoard>(TargetActor))
		{
			Board->ServerCycleNext();
		}
		break;
	case EStationButtonAction::BoardAccept:
		if (ALRAlertBoard* Board = Cast<ALRAlertBoard>(TargetActor))
		{
			Board->ServerAcceptCurrent();
		}
		break;
	case EStationButtonAction::WorkshopRepair:
		if (ALRWorkshop* Workshop = Cast<ALRWorkshop>(TargetActor))
		{
			Workshop->ServerRepair();
		}
		break;
	default:
		if (ALRWorkshop* Workshop = Cast<ALRWorkshop>(TargetActor))
		{
			Workshop->ServerBuy(Interactor);
		}
		break;
	}
}
