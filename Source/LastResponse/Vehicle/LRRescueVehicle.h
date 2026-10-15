#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "LRRescueVehicle.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USpotLightComponent;
class UAudioComponent;
class USoundBase;
class UInputMappingContext;
class UInputAction;
class ULRInteractableComponent;
class ULRVehicleSeatInteractableComponent;
class ALRPlayerCharacter;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRConditionChangedEvent, float, NewCondition);

UCLASS()
class LASTRESPONSE_API ALRRescueVehicle : public APawn
{
	GENERATED_BODY()

public:
	ALRRescueVehicle();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	float GetCondition() const { return Condition; }

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	bool HasDriver() const { return DriverCharacter.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	bool HasFreeSeat() const { return OccupantCount < 1 + PassengerSeatOffsets.Num(); }

	bool IsOccupant(const ALRPlayerCharacter* Player) const;
	void ForceEjectOccupant(ALRPlayerCharacter* Occupant);

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void RepairToFull();

	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void ApplyStoredCondition();

	UPROPERTY(BlueprintAssignable, Category = "Vehicle")
	FLRConditionChangedEvent OnConditionChanged;

	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
	void BP_OnConditionChanged(float NewCondition);

	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
	void BP_OnSirenToggled(bool bSirenOn);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Drive(const FInputActionValue& Value);
	void RequestExit();
	void ToggleSiren();

	UFUNCTION(Server, Unreliable)
	void ServerSetDriveInput(FVector2D DriveInput);

	UFUNCTION(Server, Reliable)
	void ServerRequestExit();

	UFUNCTION(Server, Reliable)
	void ServerToggleSiren();

	UFUNCTION()
	void HandleEnterCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor);

	UFUNCTION()
	void OnRep_Condition();

	UFUNCTION()
	void OnRep_SirenOn();

	void AttachRider(ALRPlayerCharacter* Rider, const FVector& SeatOffset);
	void DetachRider(ALRPlayerCharacter* Rider, bool bRestorePossession);
	void ServerDamage(float Amount);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> VehicleRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CabinMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SirenMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<UStaticMeshComponent>> WheelMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> ChaseCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> SirenLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRVehicleSeatInteractableComponent> EnterComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> VehicleMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DriveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ExitAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SirenAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float MaxForwardSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float MaxReverseSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float Acceleration = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float MaxYawRate = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector DriverSeatOffset = FVector(0.f, -40.f, 120.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	TArray<FVector> PassengerSeatOffsets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector ExitOffset = FVector(0.f, 300.f, 100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float DamageSpeedThreshold = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle")
	TObjectPtr<USoundBase> SirenSound;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Condition, BlueprintReadOnly, Category = "Vehicle")
	float Condition = 100.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_SirenOn, BlueprintReadOnly, Category = "Vehicle")
	bool bSirenOn = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vehicle")
	int32 OccupantCount = 0;

private:
	void IntegrateDrive(float DeltaTime);
	void UpdateOccupantCount();

	TWeakObjectPtr<ALRPlayerCharacter> DriverCharacter;
	TWeakObjectPtr<APlayerController> DriverController;
	TArray<TWeakObjectPtr<ALRPlayerCharacter>> PassengerCharacters;

	FVector2D PendingDriveInput = FVector2D::ZeroVector;
	float CurrentSpeed = 0.f;
	float SirenSpinAngle = 0.f;
};
