#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRStationButton.generated.h"

class UStaticMeshComponent;
class ULRInteractableComponent;
class ALRPlayerCharacter;

UENUM(BlueprintType)
enum class EStationButtonAction : uint8
{
	BoardNext,
	BoardAccept,
	WorkshopRepair,
	WorkshopBuy
};

UCLASS()
class LASTRESPONSE_API ALRStationButton : public AActor
{
	GENERATED_BODY()

public:
	ALRStationButton();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInteractionCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ButtonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRInteractableComponent> Interactable;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Station")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Station")
	EStationButtonAction Action = EStationButtonAction::BoardNext;
};
