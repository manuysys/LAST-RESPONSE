#include "Vehicle/LRVehicleSeatInteractableComponent.h"
#include "Vehicle/LRRescueVehicle.h"

bool ULRVehicleSeatInteractableComponent::CanInteractWith_Implementation(const ALRPlayerCharacter* Interactor) const
{
	const ALRRescueVehicle* Vehicle = Cast<ALRRescueVehicle>(GetOwner());
	return Vehicle && Vehicle->HasFreeSeat();
}
