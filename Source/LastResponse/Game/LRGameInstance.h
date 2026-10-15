#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Meta/LRMetaTypes.h"
#include "LRGameInstance.generated.h"

UCLASS()
class LASTRESPONSE_API ULRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SetActiveContract(const FContract& NewContract);

	UFUNCTION(BlueprintPure, Category = "Session")
	FContract GetActiveContract() const { return ActiveContract; }

	UFUNCTION(BlueprintPure, Category = "Session")
	bool HasActiveContract() const { return ActiveContract.ContractId != NAME_None; }

	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostMission(const FContract& Contract);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinServer(const FString& Address);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void ReturnToStation();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	FName StationMapName = TEXT("Station");

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	FContract ActiveContract;
};
