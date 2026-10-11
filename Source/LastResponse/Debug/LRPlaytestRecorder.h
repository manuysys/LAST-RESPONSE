#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LRPlaytestRecorder.generated.h"

UCLASS()
class LASTRESPONSE_API ULRPlaytestRecorder : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Playtest")
	void BeginSession(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "Playtest")
	void RecordEvent(const FString& Category, const FString& Detail);

	UFUNCTION(BlueprintCallable, Category = "Playtest")
	void EndSession(const FString& Summary);

private:
	FString BuildFilePath() const;

	TArray<FString> Lines;
	FString SessionMap;
	FString SessionTimestamp;
	bool bSessionActive = false;
};
