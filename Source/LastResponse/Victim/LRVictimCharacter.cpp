#include "Victim/LRVictimCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
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
	GetCharacterMovement()->MaxWalkSpeed = FollowSpeed;

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
}

void ALRVictimCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = FollowSpeed;

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
}

void ALRVictimCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRVictimCharacter, VictimState);
}

void ALRVictimCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (VictimState != ELRVictimState::Assisted || !EscortTarget.IsValid())
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

void ALRVictimCharacter::OnRep_VictimState()
{
	if (AssistComponent)
	{
		AssistComponent->bEnabled = VictimState != ELRVictimState::Evacuated;
	}

	if (VictimState == ELRVictimState::Assisted && !EscortTarget.IsValid())
	{
		if (ALRPlayerCharacter* Player = Cast<ALRPlayerCharacter>(GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr))
		{
			EscortTarget = Player;
		}
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
