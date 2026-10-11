#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LRMetaLibrary.generated.h"

UCLASS()
class LASTRESPONSE_API ULRMetaLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Meta")
	static float GetToolWeight(FName ToolId);

	UFUNCTION(BlueprintPure, Category = "Meta")
	static int32 GetToolPrice(FName ToolId);
};
