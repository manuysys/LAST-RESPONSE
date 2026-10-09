#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRBlockedDoor.generated.h"

class UStaticMeshComponent;
class ULRInteractableComponent;
class ALRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRDoorOpenedEvent);

UCLASS()
class LASTRESPONSE_API ALRBlockedDoor : public AActor
{
	GENERATED_BODY()

public:
	ALRBlockedDoor();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsOpen() const { return bOpen; }

	UPROPERTY(BlueprintAssignable, Category = "Door")
	FLRDoorOpenedEvent OnDoorOpened;

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void BP_OnDoorOpened();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInteractionCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenAngle = 95.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float OpenDuration = 1.f;

private:
	bool bOpen = false;
	bool bAnimating = false;
	float OpenAlpha = 0.f;
};
