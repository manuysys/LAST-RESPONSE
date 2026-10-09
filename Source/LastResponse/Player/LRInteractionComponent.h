#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LRInteractionComponent.generated.h"

class ALRPlayerCharacter;
class ULRInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRFocusChangedEvent, ULRInteractableComponent*, NewFocus);

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULRInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float TraceDistance = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float TraceRadius = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float TraceInterval = 0.1f;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FLRFocusChangedEvent OnFocusChanged;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void BeginInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void EndInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	ULRInteractableComponent* GetFocusedInteractable() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetHoldProgress() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsHolding() const { return bHolding; }

private:
	ALRPlayerCharacter* GetOwnerPlayer() const;
	void UpdateFocus();
	bool TraceForInteractable(FHitResult& OutHit) const;

	TWeakObjectPtr<ULRInteractableComponent> FocusedInteractable;
	TWeakObjectPtr<ULRInteractableComponent> ActiveInteractable;

	float HoldElapsed = 0.f;
	float TimeSinceTrace = 0.f;
	bool bHolding = false;
};
