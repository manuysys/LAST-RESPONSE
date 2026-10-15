#pragma once

#include "CoreMinimal.h"
#include "Interaction/LRInteractableComponent.h"
#include "LRVehicleSeatInteractableComponent.generated.h"

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRVehicleSeatInteractableComponent : public ULRInteractableComponent
{
	GENERATED_BODY()

public:
	virtual bool CanInteractWith_Implementation(const ALRPlayerCharacter* Interactor) const override;
};
