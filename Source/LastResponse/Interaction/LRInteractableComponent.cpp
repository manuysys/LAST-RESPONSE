#include "Interaction/LRInteractableComponent.h"
#include "Engine/World.h"
#include "Game/LRGameMode.h"
#include "Game/LRMissionDirector.h"
#include "Player/LRPlayerCharacter.h"
#include "Player/LRInventoryComponent.h"

ULRInteractableComponent::ULRInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool ULRInteractableComponent::CanInteract(const ALRPlayerCharacter* Interactor) const
{
	if (!bEnabled || !Interactor)
	{
		return false;
	}

	if (RequiredToolId != NAME_None)
	{
		const ULRInventoryComponent* Inventory = Interactor->GetInventoryComponent();
		if (!Inventory || !Inventory->HasTool(RequiredToolId))
		{
			return false;
		}
	}

	return CanInteractWith(Interactor);
}

void ULRInteractableComponent::CompleteInteract(ALRPlayerCharacter* Interactor)
{
	if (RequiredToolId != NAME_None && GetWorld())
	{
		if (ALRGameMode* GameMode = GetWorld()->GetAuthGameMode<ALRGameMode>())
		{
			if (ALRMissionDirector* Director = GameMode->GetMissionDirector())
			{
				Director->RegisterToolUse(RequiredToolId);
			}
		}
	}

	OnInteractionCompletedEvent(Interactor);
	OnInteractionCompleted.Broadcast(this, Interactor);
}

FText ULRInteractableComponent::GetPromptText() const
{
	return PromptText;
}

bool ULRInteractableComponent::CanInteractWith_Implementation(const ALRPlayerCharacter* Interactor) const
{
	return true;
}

void ULRInteractableComponent::OnInteractionCompletedEvent_Implementation(ALRPlayerCharacter* Interactor)
{
}
