#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LRInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRInventoryChangedEvent, FName, ToolId);

UCLASS(ClassGroup = (LastResponse), meta = (BlueprintSpawnableComponent))
class LASTRESPONSE_API ULRInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FName> ToolIds;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FLRInventoryChangedEvent OnToolAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FLRInventoryChangedEvent OnToolRemoved;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasTool(FName ToolId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddTool(FName ToolId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveTool(FName ToolId);
};
