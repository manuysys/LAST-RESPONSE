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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float MaxServerDistance = 450.f;

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
	bool IsHolding() const { return bHolding || bServerHolding; }

	ULRInteractableComponent* GetServerActiveTarget() const;

protected:
	UFUNCTION(Server, Reliable)
	void ServerBeginInteract(ULRInteractableComponent* Target);

	UFUNCTION(Server, Reliable)
	void ServerEndInteract();

	UFUNCTION(Client, Reliable)
	void ClientCancelInteract();

private:
	ALRPlayerCharacter* GetOwnerPlayer() const;
	bool HasAuthority() const;
	void UpdateFocus();
	bool TraceForInteractable(FHitResult& OutHit) const;

	bool IsValidInteractTarget(ULRInteractableComponent* Target) const;
	void StartLocalHold(ULRInteractableComponent* Target);
	void ClearLocalHold();
	void StartServerHold(ULRInteractableComponent* Target);
	void ClearServerHold();
	void TickLocalHold(float DeltaTime);
	void TickServerHold(float DeltaTime);

	TWeakObjectPtr<ULRInteractableComponent> FocusedInteractable;
	TWeakObjectPtr<ULRInteractableComponent> ActiveInteractable;
	TWeakObjectPtr<ULRInteractableComponent> ServerActiveTarget;

	float HoldElapsed = 0.f;
	float ServerHoldElapsed = 0.f;
	float TimeSinceTrace = 0.f;
	bool bHolding = false;
	bool bServerHolding = false;
};
