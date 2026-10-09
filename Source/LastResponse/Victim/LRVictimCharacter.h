#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LRTypes.h"
#include "LRVictimCharacter.generated.h"

class ULRVictimInteractableComponent;
class USphereComponent;
class ALRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLRVictimStateChangedEvent, ALRVictimCharacter*, Victim, ELRVictimState, NewState);

UCLASS()
class LASTRESPONSE_API ALRVictimCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALRVictimCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Victim")
	ELRVictimState GetVictimState() const { return VictimState; }

	UFUNCTION(BlueprintCallable, Category = "Victim")
	void SetVictimState(ELRVictimState NewState);

	UFUNCTION(BlueprintCallable, Category = "Victim")
	void AdvanceState();

	UFUNCTION(BlueprintPure, Category = "Victim")
	bool IsSafe();

	UFUNCTION(BlueprintCallable, Category = "Victim")
	void SetEscortTarget(ALRPlayerCharacter* NewTarget);

	UPROPERTY(BlueprintAssignable, Category = "Victim")
	FLRVictimStateChangedEvent OnVictimStateChanged;

	UFUNCTION(BlueprintImplementableEvent, Category = "Victim")
	void BP_OnVictimStateChanged(ELRVictimState NewState);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleAssistCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UFUNCTION()
	void HandleDetectionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleReachBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRVictimInteractableComponent> AssistComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> DetectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> ReachSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Victim")
	float DetectionRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Victim")
	float ReachRadius = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Victim")
	float EscortDistance = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Victim")
	float FollowSpeed = 150.f;

	UPROPERTY(ReplicatedUsing = OnRep_VictimState, BlueprintReadOnly, Category = "Victim")
	ELRVictimState VictimState = ELRVictimState::Unlocated;

	UFUNCTION()
	void OnRep_VictimState();

private:
	TWeakObjectPtr<ALRPlayerCharacter> EscortTarget;
};
