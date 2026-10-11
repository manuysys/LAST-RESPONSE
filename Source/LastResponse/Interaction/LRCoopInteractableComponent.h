#pragma once

#include "CoreMinimal.h"
#include "Interaction/LRInteractableComponent.h"
#include "LRCoopInteractableComponent.generated.h"

class ALRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLRCoopProgressEvent, int32, ActiveHolders, int32, RequiredHolders);

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRCoopInteractableComponent : public ULRInteractableComponent
{
	GENERATED_BODY()

public:
	ULRCoopInteractableComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coop", meta = (ClampMin = "2"))
	int32 RequiredHolders = 2;

	UPROPERTY(BlueprintAssignable, Category = "Coop")
	FLRCoopProgressEvent OnHolderCountChanged;

	UFUNCTION(BlueprintPure, Category = "Coop")
	int32 GetActiveHolderCount() const { return HolderCount; }

	UFUNCTION(BlueprintPure, Category = "Coop")
	float GetCoopProgress() const;

	void ServerAddHolder(ALRPlayerCharacter* Player);
	void ServerRemoveHolder(ALRPlayerCharacter* Player);

protected:
	UFUNCTION()
	void OnRep_HolderCount();

	UFUNCTION()
	void OnRep_CoopProgress();

	UPROPERTY(ReplicatedUsing = OnRep_HolderCount, BlueprintReadOnly, Category = "Coop")
	int32 HolderCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CoopProgress, BlueprintReadOnly, Category = "Coop")
	float CoopProgress = 0.f;

private:
	void PruneHolders();
	bool IsHolderActive(ALRPlayerCharacter* Player) const;

	TArray<TWeakObjectPtr<ALRPlayerCharacter>> Holders;
};
