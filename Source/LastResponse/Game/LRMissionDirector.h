#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRTypes.h"
#include "LRMissionDirector.generated.h"

class ALRVictimCharacter;
class ALRWaterDirector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRMissionEndedEvent, FMissionReport, Report);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLRMissionStartedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRMissionUpdatedEvent, FMissionReport, Report);

UCLASS()
class LASTRESPONSE_API ALRMissionDirector : public AActor
{
	GENERATED_BODY()

public:
	ALRMissionDirector();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void StartMission();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void EndMission(ELRMissionEndReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void FailMission(ELRMissionEndReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void RegisterToolUse(FName ToolId);

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void CompleteObjective(FName ObjectiveId);

	UFUNCTION(BlueprintPure, Category = "Mission")
	FMissionReport GetReport() const { return Report; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsMissionActive() const { return bStarted && Report.EndReason == ELRMissionEndReason::InProgress; }

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FLRMissionEndedEvent OnMissionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FLRMissionStartedEvent OnMissionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Mission")
	FLRMissionUpdatedEvent OnMissionUpdated;

	UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
	void BP_OnMissionEnded(const FMissionReport& EndedReport);

	UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
	void BP_OnMissionUpdated(const FMissionReport& CurrentReport);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleVictimStateChanged(ALRVictimCharacter* Victim, ELRVictimState NewState);

	UFUNCTION()
	void HandleWaterLevelChanged(float NewLevel);

	UFUNCTION()
	void OnRep_Report();

	UFUNCTION()
	void OnRep_Started();

	void CheckTeamWipe();
	void ComputeRewards();
	float ReadVehicleDamage() const;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mission")
	TArray<TObjectPtr<ALRVictimCharacter>> Victims;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Mission")
	TObjectPtr<ALRWaterDirector> WaterDirector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TArray<FName> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float TimeLimitSeconds = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bStartWaterOnMissionStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName ContractId = NAME_None;

	UPROPERTY(ReplicatedUsing = OnRep_Report, BlueprintReadOnly, Category = "Mission")
	FMissionReport Report;

	UPROPERTY(ReplicatedUsing = OnRep_Started, BlueprintReadOnly, Category = "Mission")
	bool bStarted = false;

private:
	TArray<FName> CompletedObjectives;
	bool bEndedNotified = false;
	float SessionElapsedSeconds = 0.f;
	float ReportElapsedTimer = 0.f;
};
