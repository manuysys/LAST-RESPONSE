#include "Player/LRPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Game/LRGameMode.h"
#include "Game/LRMissionDirector.h"
#include "Player/LRInteractionComponent.h"
#include "Player/LRInventoryComponent.h"

ALRPlayerCharacter::ALRPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	CameraComponent->bUsePawnControlRotation = true;

	FlashlightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightComponent"));
	FlashlightComponent->SetupAttachment(CameraComponent);
	FlashlightComponent->SetRelativeLocation(FVector(10.f, 8.f, -8.f));
	FlashlightComponent->SetVisibility(false);
	FlashlightComponent->Intensity = 5000.f;
	FlashlightComponent->InnerConeAngle = 18.f;
	FlashlightComponent->OuterConeAngle = 32.f;

	InteractionComponent = CreateDefaultSubobject<ULRInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<ULRInventoryComponent>(TEXT("InventoryComponent"));

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ALRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	bIncapacitated = false;
	RopeAssistCount = 0;
	WaterImmersion = 0.f;
	RefreshMaxWalkSpeed();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ALRPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (WaterImmersion >= 0.98f && !bIncapacitated)
	{
		bIncapacitated = true;

		if (ALRGameMode* GameMode = GetWorld()->GetAuthGameMode<ALRGameMode>())
		{
			if (ALRMissionDirector* Director = GameMode->GetMissionDirector())
			{
				Director->FailMission(ELRMissionEndReason::PlayerIncapacitated);
			}
		}
	}
}

void ALRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALRPlayerCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALRPlayerCharacter::Look);
		}

		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ALRPlayerCharacter::StartInteract);
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Completed, this, &ALRPlayerCharacter::StopInteract);
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Canceled, this, &ALRPlayerCharacter::StopInteract);
		}

		if (FlashlightAction)
		{
			EnhancedInput->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ALRPlayerCharacter::ToggleFlashlight);
		}

		if (CrouchAction)
		{
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ALRPlayerCharacter::ToggleCrouch);
		}

		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ALRPlayerCharacter::StartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ALRPlayerCharacter::StopSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ALRPlayerCharacter::StopSprint);
		}
	}
}

void ALRPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero())
	{
		return;
	}

	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ALRPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ALRPlayerCharacter::StartInteract()
{
	if (InteractionComponent)
	{
		InteractionComponent->BeginInteract();
	}
}

void ALRPlayerCharacter::StopInteract()
{
	if (InteractionComponent)
	{
		InteractionComponent->EndInteract();
	}
}

void ALRPlayerCharacter::ToggleFlashlight()
{
	if (FlashlightComponent)
	{
		FlashlightComponent->ToggleVisibility();
	}
}

void ALRPlayerCharacter::ToggleCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ALRPlayerCharacter::StartSprint()
{
	bSprinting = true;
	RefreshMaxWalkSpeed();
}

void ALRPlayerCharacter::StopSprint()
{
	bSprinting = false;
	RefreshMaxWalkSpeed();
}

void ALRPlayerCharacter::SetWaterImmersion(float ImmersionRatio)
{
	const float ClampedRatio = FMath::Clamp(ImmersionRatio, 0.f, 1.f);
	if (FMath::IsNearlyEqual(WaterImmersion, ClampedRatio))
	{
		return;
	}

	WaterImmersion = ClampedRatio;
	RefreshMaxWalkSpeed();
	BP_OnWaterImmersionChanged(WaterImmersion);
}

void ALRPlayerCharacter::AddRopeAssist()
{
	RopeAssistCount++;
}

void ALRPlayerCharacter::RemoveRopeAssist()
{
	RopeAssistCount = FMath::Max(RopeAssistCount - 1, 0);
}

void ALRPlayerCharacter::RefreshMaxWalkSpeed()
{
	const float BaseSpeed = bSprinting ? SprintSpeed : WalkSpeed;
	const float Multiplier = FMath::Lerp(1.f, MaxWaterSpeedMultiplier, WaterImmersion);
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * Multiplier;
}
