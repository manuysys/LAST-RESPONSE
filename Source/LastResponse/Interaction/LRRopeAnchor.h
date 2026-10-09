#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRRopeAnchor.generated.h"

class UStaticMeshComponent;
class ULRInteractableComponent;
class ALRRopeLine;
class ALRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRRopeSecuredEvent);

UCLASS()
class LASTRESPONSE_API ALRRopeAnchor : public AActor
{
	GENERATED_BODY()

public:
	ALRRopeAnchor();

	UFUNCTION(BlueprintPure, Category = "Rope")
	bool IsSecured() const { return bSecured; }

	UPROPERTY(BlueprintAssignable, Category = "Rope")
	FLRRopeSecuredEvent OnRopeSecured;

	UFUNCTION(BlueprintImplementableEvent, Category = "Rope")
	void BP_OnRopeSecured();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleInteractionCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> AnchorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRInteractableComponent> Interactable;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Rope")
	TObjectPtr<ALRRopeAnchor> LinkedAnchor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Rope")
	TSubclassOf<ALRRopeLine> RopeLineClass;

private:
	bool bSecured = false;
};
