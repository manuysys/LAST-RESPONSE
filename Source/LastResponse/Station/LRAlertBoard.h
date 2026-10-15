#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Meta/LRMetaTypes.h"
#include "LRAlertBoard.generated.h"

class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRContractSelectedEvent, int32, ContractIndex);

UCLASS()
class LASTRESPONSE_API ALRAlertBoard : public AActor
{
	GENERATED_BODY()

public:
	ALRAlertBoard();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Station")
	TArray<FContract> GetContracts() const { return Contracts; }

	UFUNCTION(BlueprintPure, Category = "Station")
	int32 GetCurrentIndex() const { return CurrentIndex; }

	UFUNCTION(BlueprintPure, Category = "Station")
	FContract GetCurrentContract() const;

	UFUNCTION(BlueprintCallable, Category = "Station")
	void ServerCycleNext();

	UFUNCTION(BlueprintCallable, Category = "Station")
	void ServerAcceptCurrent();

	UPROPERTY(BlueprintAssignable, Category = "Station")
	FLRContractSelectedEvent OnContractSelected;

	UFUNCTION(BlueprintImplementableEvent, Category = "Station")
	void BP_OnContractSelected(int32 ContractIndex);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentIndex();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BoardMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	TArray<FContract> Contracts;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CurrentIndex, BlueprintReadOnly, Category = "Station")
	int32 CurrentIndex = 0;
};
