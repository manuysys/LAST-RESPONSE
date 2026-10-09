#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LRInteractableComponent.generated.h"

class ALRPlayerCharacter;
class ULRInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLRInteractionEvent, ULRInteractableComponent*, Interactable, ALRPlayerCharacter*, Interactor);

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULRInteractableComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
	float HoldDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName RequiredToolId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bEnabled = true;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FLRInteractionEvent OnInteractionCompleted;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool CanInteract(const ALRPlayerCharacter* Interactor) const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CompleteInteract(ALRPlayerCharacter* Interactor);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetPromptText() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanInteractWith(const ALRPlayerCharacter* Interactor) const;
	virtual bool CanInteractWith_Implementation(const ALRPlayerCharacter* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnInteractionCompletedEvent(ALRPlayerCharacter* Interactor);
	virtual void OnInteractionCompletedEvent_Implementation(ALRPlayerCharacter* Interactor);
};
