#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "LRPlayerCharacter.generated.h"

class UCameraComponent;
class USpotLightComponent;
class UInputMappingContext;
class UInputAction;
class ULRInteractionComponent;
class ULRInventoryComponent;

UCLASS()
class LASTRESPONSE_API ALRPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALRPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Player")
	ULRInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintPure, Category = "Player")
	ULRInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Player")
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetWaterImmersion(float ImmersionRatio);

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetWaterImmersion() const { return WaterImmersion; }

	UFUNCTION(BlueprintCallable, Category = "Player")
	void AddRopeAssist();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void RemoveRopeAssist();

	UFUNCTION(BlueprintPure, Category = "Player")
	bool HasRopeAssist() const { return RopeAssistCount > 0; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Player")
	void BP_OnWaterImmersionChanged(float ImmersionRatio);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartInteract();
	void StopInteract();
	void ToggleFlashlight();
	void ToggleCrouch();
	void StartSprint();
	void StopSprint();

	void RefreshMaxWalkSpeed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpotLightComponent> FlashlightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULRInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FlashlightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 520.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MaxWaterSpeedMultiplier = 0.35f;

private:
	float WaterImmersion = 0.f;
	int32 RopeAssistCount = 0;
	bool bSprinting = false;
	bool bIncapacitated = false;
};
