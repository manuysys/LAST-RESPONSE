#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LRInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRInventoryChangedEvent, FName, ToolId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRInventoryUpdatedEvent);

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULRInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_ToolIds, BlueprintReadWrite, Category = "Inventory")
	TArray<FName> ToolIds;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FLRInventoryChangedEvent OnToolAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FLRInventoryChangedEvent OnToolRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FLRInventoryUpdatedEvent OnInventoryUpdated;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasTool(FName ToolId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddTool(FName ToolId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveTool(FName ToolId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetTools(const TArray<FName>& NewTools);

protected:
	UFUNCTION()
	void OnRep_ToolIds();
};
