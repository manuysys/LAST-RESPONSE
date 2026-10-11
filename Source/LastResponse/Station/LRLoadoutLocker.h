#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRLoadoutLocker.generated.h"

class UStaticMeshComponent;
class ULRInteractableComponent;
class ALRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRLoadoutAppliedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRLoadoutRejectedEvent, float, RequiredWeight);

UCLASS()
class LASTRESPONSE_API ALRLoadoutLocker : public AActor
{
	GENERATED_BODY()

public:
	ALRLoadoutLocker();

	UPROPERTY(BlueprintAssignable, Category = "Station")
	FLRLoadoutAppliedEvent OnLoadoutApplied;

	UPROPERTY(BlueprintAssignable, Category = "Station")
	FLRLoadoutRejectedEvent OnLoadoutRejected;

	UFUNCTION(BlueprintImplementableEvent, Category = "Station")
	void BP_OnLoadoutApplied();

	UFUNCTION(BlueprintImplementableEvent, Category = "Station")
	void BP_OnLoadoutRejected(float RequiredWeight);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInteractionCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LockerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	TArray<FName> LoadoutTools = { TEXT("PryBar"), TEXT("Rope"), TEXT("Medkit") };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	float MaxLoadoutWeight = 8.f;
};
