#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRHeavyDebris.generated.h"

class UStaticMeshComponent;
class ULRCoopInteractableComponent;
class ALRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRDebrisClearedEvent);

UCLASS()
class LASTRESPONSE_API ALRHeavyDebris : public AActor
{
	GENERATED_BODY()

public:
	ALRHeavyDebris();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Debris")
	bool IsCleared() const { return bCleared; }

	UPROPERTY(BlueprintAssignable, Category = "Debris")
	FLRDebrisClearedEvent OnDebrisCleared;

	UFUNCTION(BlueprintImplementableEvent, Category = "Debris")
	void BP_OnDebrisCleared();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInteractionCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UFUNCTION()
	void OnRep_Cleared();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DebrisMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRCoopInteractableComponent> CoopInteractable;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Cleared)
	bool bCleared = false;
};
