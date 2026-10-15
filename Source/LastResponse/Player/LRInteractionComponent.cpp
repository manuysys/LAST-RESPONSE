#include "Player/LRInteractionComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Interaction/LRInteractableComponent.h"
#include "Interaction/LRCoopInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"

ULRInteractionComponent::ULRInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void ULRInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceTrace += DeltaTime;
	if (TimeSinceTrace >= TraceInterval)
	{
		TimeSinceTrace = 0.f;
		UpdateFocus();
	}

	if (HasAuthority())
	{
		TickServerHold(DeltaTime);
	}
	else
	{
		TickLocalHold(DeltaTime);
	}
}

void ULRInteractionComponent::BeginInteract()
{
	ALRPlayerCharacter* Player = GetOwnerPlayer();
	ULRInteractableComponent* Target = FocusedInteractable.Get();

	if (!Player || !Target || !Target->CanInteract(Player))
	{
		return;
	}

	if (HasAuthority())
	{
		StartServerHold(Target);
		return;
	}

	StartLocalHold(Target);
	ServerBeginInteract(Target);
}

void ULRInteractionComponent::EndInteract()
{
	ClearLocalHold();

	if (!HasAuthority())
	{
		ServerEndInteract();
	}
	else
	{
		ClearServerHold();
	}
}

ULRInteractableComponent* ULRInteractionComponent::GetFocusedInteractable() const
{
	return FocusedInteractable.Get();
}

ULRInteractableComponent* ULRInteractionComponent::GetServerActiveTarget() const
{
	return ServerActiveTarget.Get();
}

float ULRInteractionComponent::GetHoldProgress() const
{
	if (bServerHolding)
	{
		if (const ULRCoopInteractableComponent* Coop = Cast<ULRCoopInteractableComponent>(ServerActiveTarget.Get()))
		{
			return Coop->GetCoopProgress();
		}

		const ULRInteractableComponent* ServerTarget = ServerActiveTarget.Get();
		if (ServerTarget && ServerTarget->HoldDuration > 0.f)
		{
			return FMath::Clamp(ServerHoldElapsed / ServerTarget->HoldDuration, 0.f, 1.f);
		}
		return 0.f;
	}

	if (const ULRCoopInteractableComponent* Coop = Cast<ULRCoopInteractableComponent>(ActiveInteractable.Get()))
	{
		return Coop->GetCoopProgress();
	}

	const ULRInteractableComponent* Active = ActiveInteractable.Get();
	if (!bHolding || !Active || Active->HoldDuration <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(HoldElapsed / Active->HoldDuration, 0.f, 1.f);
}

void ULRInteractionComponent::ServerBeginInteract_Implementation(ULRInteractableComponent* Target)
{
	ALRPlayerCharacter* Player = GetOwnerPlayer();
	if (!Player || !IsValidInteractTarget(Target))
	{
		ClientCancelInteract();
		return;
	}

	StartServerHold(Target);
}

void ULRInteractionComponent::ServerEndInteract_Implementation()
{
	ClearServerHold();
}

void ULRInteractionComponent::ClientCancelInteract_Implementation()
{
	ClearLocalHold();
}

ALRPlayerCharacter* ULRInteractionComponent::GetOwnerPlayer() const
{
	return Cast<ALRPlayerCharacter>(GetOwner());
}

bool ULRInteractionComponent::HasAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}

void ULRInteractionComponent::UpdateFocus()
{
	ULRInteractableComponent* NewFocus = nullptr;
	FHitResult Hit;

	if (TraceForInteractable(Hit))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			ALRPlayerCharacter* Player = GetOwnerPlayer();
			TArray<ULRInteractableComponent*> Components;
			HitActor->GetComponents<ULRInteractableComponent>(Components);
			for (ULRInteractableComponent* Candidate : Components)
			{
				if (Candidate && Candidate->CanInteract(Player))
				{
					NewFocus = Candidate;
					break;
				}
			}
			if (!NewFocus && Components.Num() > 0)
			{
				NewFocus = Components[0];
			}
		}
	}

	if (NewFocus != FocusedInteractable.Get())
	{
		FocusedInteractable = NewFocus;
		OnFocusChanged.Broadcast(NewFocus);
	}
}

bool ULRInteractionComponent::TraceForInteractable(FHitResult& OutHit) const
{
	const ALRPlayerCharacter* Player = GetOwnerPlayer();
	if (!Player)
	{
		return false;
	}

	const UCameraComponent* Camera = Player->GetCameraComponent();
	if (!Camera)
	{
		return false;
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * TraceDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	return GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TraceRadius), Params);
}

bool ULRInteractionComponent::IsValidInteractTarget(ULRInteractableComponent* Target) const
{
	ALRPlayerCharacter* Player = GetOwnerPlayer();
	if (!Player || !Target || !Target->GetOwner())
	{
		return false;
	}

	if (!Target->CanInteract(Player))
	{
		return false;
	}

	const float Distance = (Target->GetOwner()->GetActorLocation() - Player->GetActorLocation()).Size();
	return Distance <= MaxServerDistance;
}

void ULRInteractionComponent::StartLocalHold(ULRInteractableComponent* Target)
{
	ActiveInteractable = Target;
	HoldElapsed = 0.f;
	bHolding = Target->HoldDuration > 0.f;
}

void ULRInteractionComponent::ClearLocalHold()
{
	bHolding = false;
	ActiveInteractable = nullptr;
	HoldElapsed = 0.f;
}

void ULRInteractionComponent::StartServerHold(ULRInteractableComponent* Target)
{
	ALRPlayerCharacter* Player = GetOwnerPlayer();
	if (!Player)
	{
		return;
	}

	ServerActiveTarget = Target;
	ServerHoldElapsed = 0.f;
	bServerHolding = true;

	if (ULRCoopInteractableComponent* Coop = Cast<ULRCoopInteractableComponent>(Target))
	{
		Coop->ServerAddHolder(Player);
		return;
	}

	if (Target->HoldDuration <= 0.f)
	{
		ClearServerHold();
		Target->CompleteInteract(Player);
	}
}

void ULRInteractionComponent::ClearServerHold()
{
	ULRInteractableComponent* Target = ServerActiveTarget.Get();
	ALRPlayerCharacter* Player = GetOwnerPlayer();

	if (Target && Player)
	{
		if (ULRCoopInteractableComponent* Coop = Cast<ULRCoopInteractableComponent>(Target))
		{
			Coop->ServerRemoveHolder(Player);
		}
	}

	bServerHolding = false;
	ServerActiveTarget = nullptr;
	ServerHoldElapsed = 0.f;
}

void ULRInteractionComponent::TickLocalHold(float DeltaTime)
{
	if (!bHolding)
	{
		return;
	}

	ULRInteractableComponent* Active = ActiveInteractable.Get();
	ALRPlayerCharacter* Player = GetOwnerPlayer();

	if (!Active || !Player || Active != FocusedInteractable.Get() || !Active->CanInteract(Player))
	{
		ClearLocalHold();
		return;
	}

	HoldElapsed += DeltaTime;
}

void ULRInteractionComponent::TickServerHold(float DeltaTime)
{
	if (!bServerHolding)
	{
		return;
	}

	ULRInteractableComponent* Target = ServerActiveTarget.Get();
	ALRPlayerCharacter* Player = GetOwnerPlayer();

	if (!Target || !Player || !Target->bEnabled || !IsValidInteractTarget(Target))
	{
		ClearServerHold();
		return;
	}

	if (Cast<ULRCoopInteractableComponent>(Target))
	{
		return;
	}

	ServerHoldElapsed += DeltaTime;
	if (ServerHoldElapsed >= FMath::Max(Target->HoldDuration, KINDA_SMALL_NUMBER))
	{
		ULRInteractableComponent* Completed = Target;
		ClearServerHold();
		Completed->CompleteInteract(Player);
	}
}
