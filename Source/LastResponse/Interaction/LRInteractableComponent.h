#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LRInteractableComponent.generated.h"

class ALRPlayerCharacter;
class ULRInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLRInteractionEvent, ULRInteractableComponent*, Interactable, ALRPlayerCharacter*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLREnabledChangedEvent, bool, bNewEnabled);

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULRInteractableComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
	float HoldDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FName RequiredToolId = NAME_None;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Enabled, BlueprintReadWrite, Category = "Interaction")
	bool bEnabled = true;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FLRInteractionEvent OnInteractionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FLREnabledChangedEvent OnEnabledChanged;

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

protected:
	UFUNCTION()
	void OnRep_Enabled();
};
