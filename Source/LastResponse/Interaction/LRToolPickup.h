#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRToolPickup.generated.h"

class UStaticMeshComponent;
class ULRInteractableComponent;
class ALRPlayerCharacter;

UCLASS()
class LASTRESPONSE_API ALRToolPickup : public AActor
{
	GENERATED_BODY()

public:
	ALRToolPickup();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInteractionCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FName ToolId = TEXT("PryBar");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	bool bDestroyOnPickup = true;
};
