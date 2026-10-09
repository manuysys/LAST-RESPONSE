#pragma once

#include "CoreMinimal.h"
#include "Interaction/LRInteractableComponent.h"
#include "LRVictimInteractableComponent.generated.h"

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRVictimInteractableComponent : public ULRInteractableComponent
{
	GENERATED_BODY()

public:
	virtual bool CanInteractWith_Implementation(const ALRPlayerCharacter* Interactor) const override;
};
