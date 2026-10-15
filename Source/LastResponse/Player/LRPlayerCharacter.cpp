#include "Player/LRPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Game/LRGameMode.h"
#include "Game/LRMissionDirector.h"
#include "Interaction/LRInteractableComponent.h"
#include "Meta/LRProgressionSubsystem.h"
#include "Player/LRInteractionComponent.h"
#include "Player/LRInventoryComponent.h"
#include "Vehicle/LRRescueVehicle.h"
#include "Victim/LRVictimCharacter.h"
#include "World/LRStormDirector.h"
#include "World/LRWaterDirector.h"

ALRPlayerCharacter::ALRPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

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

	PrototypeBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBodyMesh"));
	PrototypeBodyMesh->SetupAttachment(GetCapsuleComponent());
	PrototypeBodyMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.76f));
	PrototypeBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeBodyMesh->SetOwnerNoSee(true);
	if (UStaticMesh* Cylinder = FLRProto::CylinderMesh())
	{
		PrototypeBodyMesh->SetStaticMesh(Cylinder);
	}

	PrototypeHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeHeadMesh"));
	PrototypeHeadMesh->SetupAttachment(GetCapsuleComponent());
	PrototypeHeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 85.f));
	PrototypeHeadMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	PrototypeHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeHeadMesh->SetOwnerNoSee(true);
	if (UStaticMesh* Sphere = FLRProto::SphereMesh())
	{
		PrototypeHeadMesh->SetStaticMesh(Sphere);
	}

	InteractionComponent = CreateDefaultSubobject<ULRInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<ULRInventoryComponent>(TEXT("InventoryComponent"));

	ReviveComponent = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("ReviveComponent"));
	ReviveComponent->HoldDuration = 4.f;
	ReviveComponent->bEnabled = false;

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ALRPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRPlayerCharacter, bDowned);
}

void ALRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	bIncapacitated = false;
	bSprinting = false;
	RopeAssistCount = 0;
	WaterImmersion = 0.f;
	RefreshMaxWalkSpeed();

	if (ReviveComponent)
	{
		ReviveComponent->OnInteractionCompleted.AddDynamic(this, &ALRPlayerCharacter::HandleReviveCompleted);
		if (ReviveComponent->PromptText.IsEmpty())
		{
			ReviveComponent->PromptText = NSLOCTEXT("LastResponse", "ReviveTeammate", "Revive teammate");
		}
	}

	const bool bRealModel = FLRProto::UpgradeToRealModel(PrototypeBodyMesh, TEXT("SM_Player"), FVector::OneVector, FVector(0.f, 0.f, -88.f));
	if (bRealModel)
	{
		PrototypeHeadMesh->SetVisibility(false);
	}
	else
	{
		FLRProto::ApplyColor(PrototypeBodyMesh, FLinearColor(0.95f, 0.42f, 0.08f));
		FLRProto::ApplyColor(PrototypeHeadMesh, FLinearColor(0.92f, 0.76f, 0.6f));
	}

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

	CheckSubmersion();
}

void ALRPlayerCharacter::CheckSubmersion()
{
	if (WaterImmersion < 0.98f || bIncapacitated || bDowned)
	{
		return;
	}

	bIncapacitated = true;

	if (GetNetMode() == NM_Standalone)
	{
		if (ALRGameMode* GameMode = GetWorld()->GetAuthGameMode<ALRGameMode>())
		{
			if (ALRMissionDirector* Director = GameMode->GetMissionDirector())
			{
				Director->FailMission(ELRMissionEndReason::PlayerIncapacitated);
			}
		}
		return;
	}

	if (HasAuthority())
	{
		bDowned = true;
		OnRep_Downed();

		for (TActorIterator<ALRRescueVehicle> It(GetWorld()); It; ++It)
		{
			if (It->IsOccupant(this))
			{
				It->ForceEjectOccupant(this);
				break;
			}
		}

		if (GetCharacterMovement())
		{
			GetCharacterMovement()->DisableMovement();
		}

		if (ReviveComponent)
		{
			ReviveComponent->bEnabled = true;
		}

		if (InteractionComponent)
		{
			InteractionComponent->EndInteract();
		}
	}
}

void ALRPlayerCharacter::HandleReviveCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor)
{
	if (!HasAuthority() || !bDowned || !Interactor || Interactor == this)
	{
		return;
	}

	bDowned = false;
	bIncapacitated = false;
	OnRep_Downed();

	const FVector SafeLocation = Interactor->GetActorLocation()
		+ Interactor->GetActorForwardVector() * ReviveTeleportDistance
		+ FVector(0.f, 0.f, 10.f);
	SetActorLocation(SafeLocation, true, nullptr, ETeleportType::TeleportPhysics);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	if (ReviveComponent)
	{
		ReviveComponent->bEnabled = false;
	}
}

void ALRPlayerCharacter::OnRep_Downed()
{
	BP_OnDownedChanged(bDowned);
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
	if (bDowned)
	{
		return;
	}

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
	if (bDowned)
	{
		return;
	}

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
	if (bDowned)
	{
		return;
	}

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

void ALRPlayerCharacter::ServerCheatGiveTool_Implementation(FName ToolId)
{
	if (InventoryComponent)
	{
		InventoryComponent->AddTool(ToolId);
	}
}

void ALRPlayerCharacter::ServerCheatTeleportToVictim_Implementation()
{
	for (TActorIterator<ALRVictimCharacter> It(GetWorld()); It; ++It)
	{
		if (!It->IsSafe())
		{
			SetActorLocation(It->GetActorLocation() + FVector(200.f, 0.f, 50.f), false, nullptr, ETeleportType::TeleportPhysics);
			return;
		}
	}
}

void ALRPlayerCharacter::ServerCheatWinMission_Implementation()
{
	for (TActorIterator<ALRMissionDirector> It(GetWorld()); It; ++It)
	{
		It->EndMission(ELRMissionEndReason::Success);
		return;
	}
}

void ALRPlayerCharacter::ServerCheatCompleteObjective_Implementation(FName ObjectiveId)
{
	for (TActorIterator<ALRMissionDirector> It(GetWorld()); It; ++It)
	{
		It->CompleteObjective(ObjectiveId);
		return;
	}
}

void ALRPlayerCharacter::ServerCheatSetStorm_Implementation(float Rain, float Wind)
{
	if (ALRStormDirector* Storm = ALRStormDirector::FindStorm(GetWorld()))
	{
		Storm->SetStorm(Rain, Wind);
	}
}

void ALRPlayerCharacter::ServerCheatSetWater_Implementation(float NewLevel)
{
	for (TActorIterator<ALRWaterDirector> It(GetWorld()); It; ++It)
	{
		It->SetWaterLevel(NewLevel);
	}
}

void ALRPlayerCharacter::ServerCheatForceRise_Implementation(bool bRise)
{
	for (TActorIterator<ALRWaterDirector> It(GetWorld()); It; ++It)
	{
		if (bRise)
		{
			It->StartRise();
		}
		else
		{
			It->StopRise();
		}
	}
}

void ALRPlayerCharacter::ServerCheatAddBudget_Implementation(int32 Amount)
{
	if (ULRProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULRProgressionSubsystem>() : nullptr)
	{
		Progression->AddBudget(Amount);
	}
}
