#include "Vehicle/LRRescueVehicle.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "Interaction/LRInteractableComponent.h"
#include "Meta/LRProgressionSubsystem.h"
#include "Player/LRPlayerCharacter.h"

ALRRescueVehicle::ALRRescueVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	SetRootComponent(BodyMesh);
	BodyMesh->SetSimulatePhysics(false);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(BodyMesh);
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	CameraBoom->TargetArmLength = 700.f;
	CameraBoom->bUsePawnControlRotation = true;

	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(CameraBoom);

	SirenLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SirenLight"));
	SirenLight->SetupAttachment(BodyMesh);
	SirenLight->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	SirenLight->SetVisibility(false);
	SirenLight->Intensity = 8000.f;
	SirenLight->InnerConeAngle = 12.f;
	SirenLight->OuterConeAngle = 30.f;

	EnterComponent = CreateDefaultSubobject<ULRInteractableComponent>(TEXT("EnterComponent"));
	EnterComponent->HoldDuration = 0.5f;

	PassengerSeatOffsets = { FVector(-80.f, 40.f, 120.f), FVector(-160.f, -40.f, 120.f), FVector(-160.f, 40.f, 120.f) };
}

void ALRRescueVehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRRescueVehicle, Condition);
	DOREPLIFETIME(ALRRescueVehicle, bSirenOn);
}

void ALRRescueVehicle::BeginPlay()
{
	Super::BeginPlay();

	ApplyStoredCondition();

	if (EnterComponent)
	{
		EnterComponent->OnInteractionCompleted.AddDynamic(this, &ALRRescueVehicle::HandleEnterCompleted);
		if (EnterComponent->PromptText.IsEmpty())
		{
			EnterComponent->PromptText = NSLOCTEXT("LastResponse", "EnterVehicle", "Enter vehicle");
		}
	}
}

void ALRRescueVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		IntegrateDrive(DeltaSeconds);
	}

	if (bSirenOn && SirenLight)
	{
		SirenSpinAngle += DeltaSeconds * 540.f;
		SirenLight->SetRelativeRotation(FRotator(-30.f, SirenSpinAngle, 0.f));
	}
}

void ALRRescueVehicle::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (HasAuthority() && FMath::Abs(CurrentSpeed) > DamageSpeedThreshold)
	{
		ServerDamage(FMath::Abs(CurrentSpeed) / 200.f);
		CurrentSpeed *= -0.2f;
	}
}

void ALRRescueVehicle::RepairToFull()
{
	Condition = 100.f;
	OnRep_Condition();
}

void ALRRescueVehicle::ApplyStoredCondition()
{
	if (!HasAuthority())
	{
		return;
	}

	if (ULRProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULRProgressionSubsystem>() : nullptr)
	{
		Condition = FMath::Clamp(Progression->GetStoredVehicleCondition(), 0.f, 100.f);
	}
}

void ALRRescueVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (VehicleMappingContext)
			{
				Subsystem->AddMappingContext(VehicleMappingContext, 1);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (DriveAction)
		{
			EnhancedInput->BindAction(DriveAction, ETriggerEvent::Triggered, this, &ALRRescueVehicle::Drive);
			EnhancedInput->BindAction(DriveAction, ETriggerEvent::Completed, this, &ALRRescueVehicle::Drive);
			EnhancedInput->BindAction(DriveAction, ETriggerEvent::Canceled, this, &ALRRescueVehicle::Drive);
		}

		if (ExitAction)
		{
			EnhancedInput->BindAction(ExitAction, ETriggerEvent::Started, this, &ALRRescueVehicle::RequestExit);
		}

		if (SirenAction)
		{
			EnhancedInput->BindAction(SirenAction, ETriggerEvent::Started, this, &ALRRescueVehicle::ToggleSiren);
		}
	}
}

void ALRRescueVehicle::Drive(const FInputActionValue& Value)
{
	ServerSetDriveInput(Value.Get<FVector2D>());
}

void ALRRescueVehicle::RequestExit()
{
	ServerRequestExit();
}

void ALRRescueVehicle::ToggleSiren()
{
	ServerToggleSiren();
}

void ALRRescueVehicle::ServerSetDriveInput_Implementation(FVector2D DriveInput)
{
	PendingDriveInput = DriveInput.GetClampedToSize(-1.f, 1.f);
}

void ALRRescueVehicle::ServerRequestExit_Implementation()
{
	ALRPlayerCharacter* Driver = DriverCharacter.Get();
	if (Driver && Driver->GetController() == GetController())
	{
		DetachRider(Driver, true);
		DriverCharacter = nullptr;
		return;
	}

	for (const TWeakObjectPtr<ALRPlayerCharacter>& Passenger : PassengerCharacters)
	{
		if (Passenger.IsValid() && Passenger->GetController() == GetController())
		{
			DetachRider(Passenger.Get(), false);
			PassengerCharacters.Remove(Passenger);
			return;
		}
	}
}

void ALRRescueVehicle::ServerToggleSiren_Implementation()
{
	bSirenOn = !bSirenOn;
	OnRep_SirenOn();
}

void ALRRescueVehicle::HandleEnterCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor)
{
	if (!HasAuthority() || !Interactor || Interactor->IsDowned())
	{
		return;
	}

	if (!DriverCharacter.IsValid())
	{
		AttachRider(Interactor, DriverSeatOffset);
		DriverCharacter = Interactor;
		DriverController = Cast<APlayerController>(Interactor->GetController());
		if (DriverController.IsValid())
		{
			DriverController->Possess(this);
		}
		return;
	}

	if (PassengerCharacters.Num() < PassengerSeatOffsets.Num())
	{
		AttachRider(Interactor, PassengerSeatOffsets[PassengerCharacters.Num()]);
		PassengerCharacters.Add(Interactor);
	}
}

void ALRRescueVehicle::OnRep_Condition()
{
	OnConditionChanged.Broadcast(Condition);
	BP_OnConditionChanged(Condition);
}

void ALRRescueVehicle::OnRep_SirenOn()
{
	if (SirenLight)
	{
		SirenLight->SetVisibility(bSirenOn);
	}

	BP_OnSirenToggled(bSirenOn);
}

void ALRRescueVehicle::AttachRider(ALRPlayerCharacter* Rider, const FVector& SeatOffset)
{
	if (!Rider)
	{
		return;
	}

	if (ULRInteractionComponent* Interaction = Rider->GetInteractionComponent())
	{
		Interaction->EndInteract();
	}

	Rider->SetActorEnableCollision(false);
	Rider->SetActorHiddenInGame(true);

	if (UCharacterMovementComponent* Movement = Rider->GetCharacterMovement())
	{
		Movement->DisableMovement();
		Movement->StopMovementImmediately();
	}

	Rider->AttachToComponent(BodyMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Rider->SetActorRelativeLocation(SeatOffset);
}

void ALRRescueVehicle::DetachRider(ALRPlayerCharacter* Rider, bool bRestorePossession)
{
	if (!Rider)
	{
		return;
	}

	Rider->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	const FVector ExitLocation = GetActorLocation() + GetActorQuat().RotateVector(ExitOffset);
	Rider->SetActorLocation(ExitLocation);
	Rider->SetActorHiddenInGame(false);
	Rider->SetActorEnableCollision(true);

	if (UCharacterMovementComponent* Movement = Rider->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	if (bRestorePossession && DriverController.IsValid())
	{
		DriverController->Possess(Rider);
		DriverController = nullptr;
	}
}

void ALRRescueVehicle::ServerDamage(float Amount)
{
	Condition = FMath::Clamp(Condition - Amount, 0.f, 100.f);
	OnRep_Condition();
}

void ALRRescueVehicle::IntegrateDrive(float DeltaTime)
{
	if (!DriverCharacter.IsValid())
	{
		CurrentSpeed = FMath::FInterpTo(CurrentSpeed, 0.f, DeltaTime, 2.f);
		PendingDriveInput = FVector2D::ZeroVector;
	}
	else
	{
		const float Throttle = FMath::Clamp(PendingDriveInput.Y, -1.f, 1.f);
		const float Steering = FMath::Clamp(PendingDriveInput.X, -1.f, 1.f);

		const float TargetMax = Throttle >= 0.f ? MaxForwardSpeed : -MaxReverseSpeed;
		CurrentSpeed = FMath::FInterpTo(CurrentSpeed, TargetMax * Throttle, DeltaTime, 1.5f);

		const float SpeedFactor = FMath::Clamp(CurrentSpeed / 300.f, -1.f, 1.f);
		const float YawDelta = Steering * MaxYawRate * SpeedFactor * DeltaTime;
		AddActorWorldRotation(FRotator(0.f, YawDelta, 0.f));
	}

	if (!FMath::IsNearlyZero(CurrentSpeed, 1.f))
	{
		const FVector PlanarForward = FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0.f).GetSafeNormal();
		FHitResult Hit;
		AddActorWorldOffset(PlanarForward * CurrentSpeed * DeltaTime, true, &Hit);
		if (Hit.bBlockingHit)
		{
			if (FMath::Abs(CurrentSpeed) > DamageSpeedThreshold)
			{
				ServerDamage(FMath::Abs(CurrentSpeed) / 200.f);
			}
			CurrentSpeed = 0.f;
		}
	}
}
