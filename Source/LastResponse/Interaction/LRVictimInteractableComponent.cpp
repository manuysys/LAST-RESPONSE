#include "Interaction/LRVictimInteractableComponent.h"
#include "Victim/LRVictimCharacter.h"

bool ULRVictimInteractableComponent::CanInteractWith_Implementation(const ALRPlayerCharacter* Interactor) const
{
	const ALRVictimCharacter* Victim = Cast<ALRVictimCharacter>(GetOwner());
	if (!Victim)
	{
		return false;
	}

	return Victim->GetVictimState() == ELRVictimState::AccessSecured;
}
