#include "Victim/LRVictimCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Interaction/LRInteractableComponent.h"
#include "Interaction/LRVictimInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"

ALRVictimCharacter::ALRVictimCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	GetCharacterMovement()->MaxWalkSpeed = BaseFollowSpeed;

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(GetCapsuleComponent());
	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionSphere->SetGenerateOverlapEvents(true);

	ReachSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ReachSphere"));
	ReachSphere->SetupAttachment(GetCapsuleComponent());
	ReachSphere->SetSphereRadius(ReachRadius);
	ReachSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ReachSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ReachSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ReachSphere->SetGenerateOverlapEvents(true);

	AssistComponent = CreateDefaultSubobject<ULRVictimInteractableComponent>(TEXT("AssistComponent"));
	AssistComponent->HoldDuration = 2.f;

	PrototypeBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBodyMesh"));
	PrototypeBodyMesh->SetupAttachment(GetCapsuleComponent());
	PrototypeBodyMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.76f));
	PrototypeBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FLRProto::CylinderMesh())
	{
		PrototypeBodyMesh->SetStaticMesh(Cylinder);
	}

	PrototypeHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeHeadMesh"));
	PrototypeHeadMesh->SetupAttachment(GetCapsuleComponent());
	PrototypeHeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 85.f));
	PrototypeHeadMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	PrototypeHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = FLRProto::SphereMesh())
	{
		PrototypeHeadMesh->SetStaticMesh(Sphere);
	}
}

void ALRVictimCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (DetectionSphere)
	{
		DetectionSphere->SetSphereRadius(DetectionRadius);
		DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ALRVictimCharacter::HandleDetectionBegin);
	}

	if (ReachSphere)
	{
		ReachSphere->SetSphereRadius(ReachRadius);
		ReachSphere->OnComponentBeginOverlap.AddDynamic(this, &ALRVictimCharacter::HandleReachBegin);
	}

	if (AssistComponent)
	{
		AssistComponent->OnInteractionCompleted.AddDynamic(this, &ALRVictimCharacter::HandleAssistCompleted);
		if (AssistComponent->PromptText.IsEmpty())
		{
			AssistComponent->PromptText = NSLOCTEXT("LastResponse", "AssistVictim", "Assist victim");
		}
	}

	ApplyConditionToComponents();

	const bool bRealModel = FLRProto::UpgradeToRealModel(PrototypeBodyMesh, TEXT("SM_Victim"), FVector::OneVector, FVector(0.f, 0.f, -88.f));
	if (bRealModel)
	{
		PrototypeHeadMesh->SetVisibility(false);
	}
	else
	{
		FLRProto::ApplyColor(PrototypeBodyMesh, FLinearColor(0.72f, 0.09f, 0.09f));
		FLRProto::ApplyColor(PrototypeHeadMesh, FLinearColor(0.92f, 0.76f, 0.6f));
	}
}

void ALRVictimCharacter::ApplyConditionToComponents()
{
	if (AssistComponent)
	{
		AssistComponent->HoldDuration = GetAssistHoldDuration();

		if (Condition == ELRVictimCondition::Critical)
		{
			AssistComponent->RequiredToolId = TEXT("Medkit");
		}
		else if (AssistComponent->RequiredToolId == TEXT("Medkit"))
		{
			AssistComponent->RequiredToolId = NAME_None;
		}
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = GetFollowSpeed();
	}
}

void ALRVictimCharacter::OnRep_Condition()
{
	ApplyConditionToComponents();
}

void ALRVictimCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRVictimCharacter, VictimState);
	DOREPLIFETIME(ALRVictimCharacter, Condition);
}

void ALRVictimCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || VictimState != ELRVictimState::Assisted || !EscortTarget.IsValid())
	{
		return;
	}

	const FVector ToTarget = EscortTarget->GetActorLocation() - GetActorLocation();
	const FVector Flat = FVector(ToTarget.X, ToTarget.Y, 0.f);
	const float Distance = Flat.Size();

	if (Distance > 1.f)
	{
		const FRotator DesiredRotation = Flat.Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaSeconds, 4.f));
	}

	if (Distance > EscortDistance)
	{
		AddMovementInput(Flat.GetSafeNormal(), 1.f);
	}
}

void ALRVictimCharacter::SetVictimState(ELRVictimState NewState)
{
	if (VictimState == NewState)
	{
		return;
	}

	VictimState = NewState;
	OnRep_VictimState();
}

void ALRVictimCharacter::AdvanceState()
{
	const uint8 NextState = FMath::Min(static_cast<uint8>(VictimState) + 1, static_cast<uint8>(ELRVictimState::Evacuated));
	SetVictimState(static_cast<ELRVictimState>(NextState));
}

bool ALRVictimCharacter::IsSafe()
{
	return VictimState == ELRVictimState::Evacuated;
}

void ALRVictimCharacter::SetEscortTarget(ALRPlayerCharacter* NewTarget)
{
	EscortTarget = NewTarget;
}

float ALRVictimCharacter::GetAssistHoldDuration() const
{
	switch (Condition)
	{
	case ELRVictimCondition::Injured:
		return 3.f;
	case ELRVictimCondition::Critical:
		return 4.f;
	case ELRVictimCondition::Trapped:
		return 5.f;
	default:
		return 2.f;
	}
}

float ALRVictimCharacter::GetFollowSpeed() const
{
	switch (Condition)
	{
	case ELRVictimCondition::Injured:
		return BaseFollowSpeed * 0.75f;
	case ELRVictimCondition::Critical:
		return BaseFollowSpeed * 0.5f;
	case ELRVictimCondition::Trapped:
		return BaseFollowSpeed * 0.6f;
	default:
		return BaseFollowSpeed;
	}
}

void ALRVictimCharacter::OnRep_VictimState()
{
	if (AssistComponent)
	{
		AssistComponent->bEnabled = VictimState != ELRVictimState::Evacuated;
	}

	OnVictimStateChanged.Broadcast(this, VictimState);
	BP_OnVictimStateChanged(VictimState);
}

void ALRVictimCharacter::HandleDetectionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || VictimState != ELRVictimState::Unlocated)
	{
		return;
	}

	if (Cast<ALRPlayerCharacter>(OtherActor))
	{
		SetVictimState(ELRVictimState::Located);
	}
}

void ALRVictimCharacter::HandleReachBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || VictimState != ELRVictimState::Located)
	{
		return;
	}

	if (Cast<ALRPlayerCharacter>(OtherActor))
	{
		SetVictimState(ELRVictimState::AccessSecured);
	}
}

void ALRVictimCharacter::HandleAssistCompleted(ULRInteractableComponent* Interactable, ALRPlayerCharacter* Interactor)
{
	if (!HasAuthority() || VictimState != ELRVictimState::AccessSecured)
	{
		return;
	}

	SetEscortTarget(Interactor);
	SetVictimState(ELRVictimState::Assisted);
}
