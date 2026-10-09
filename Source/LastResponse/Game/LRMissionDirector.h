#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRTypes.h"
#include "LRMissionDirector.generated.h"

class ALRVictimCharacter;
class ALRWaterDirector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRMissionEndedEvent, FMissionReport, Report);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRMissionStartedEvent);

UCLASS()
class LASTRESPONSE_API ALRMissionDirector : public AActor
{
	GENERATED_BODY()

public:
	ALRMissionDirector();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void StartMission();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void EndMission(ELRMissionEndReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void FailMission(ELRMissionEndReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void RegisterToolUse(FName ToolId);

	UFUNCTION(BlueprintPure, Category = "Mission")
	FMissionReport GetReport() const { return Report; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsMissionActive() const { return bStarted && Report.EndReason == ELRMissionEndReason::InProgress; }

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FLRMissionEndedEvent OnMissionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FLRMissionStartedEvent OnMissionStarted;

	UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
	void BP_OnMissionEnded(const FMissionReport& EndedReport);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleVictimStateChanged(ALRVictimCharacter* Victim, ELRVictimState NewState);

	UFUNCTION()
	void HandleWaterLevelChanged(float NewLevel);

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mission")
	TArray<TObjectPtr<ALRVictimCharacter>> Victims;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mission")
	TObjectPtr<ALRWaterDirector> WaterDirector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float TimeLimitSeconds = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bStartWaterOnMissionStart = true;

	UPROPERTY(BlueprintReadOnly, Category = "Mission")
	FMissionReport Report;

private:
	bool bStarted = false;
};
