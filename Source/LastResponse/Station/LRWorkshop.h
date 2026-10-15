#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRWorkshop.generated.h"

class UStaticMeshComponent;
class ALRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRWorkshopEvent);

UCLASS()
class LASTRESPONSE_API ALRWorkshop : public AActor
{
	GENERATED_BODY()

public:
	ALRWorkshop();

	UFUNCTION(BlueprintCallable, Category = "Station")
	void ServerRepair();

	UFUNCTION(BlueprintCallable, Category = "Station")
	void ServerBuy(ALRPlayerCharacter* Buyer);

	UPROPERTY(BlueprintAssignable, Category = "Station")
	FLRWorkshopEvent OnRepairCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Station")
	FLRWorkshopEvent OnRepairDenied;

	UPROPERTY(BlueprintAssignable, Category = "Station")
	FLRWorkshopEvent OnPurchaseCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Station")
	FLRWorkshopEvent OnPurchaseDenied;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WorkshopMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	int32 RepairCost = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	int32 RestockCost = 120;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	TArray<FName> RestockTools = { TEXT("Medkit"), TEXT("Rope") };
};
