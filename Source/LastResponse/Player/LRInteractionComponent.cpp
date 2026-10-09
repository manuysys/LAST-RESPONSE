#include "Player/LRInteractionComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Interaction/LRInteractableComponent.h"
#include "Player/LRPlayerCharacter.h"

ULRInteractionComponent::ULRInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

	if (!bHolding)
	{
		return;
	}

	ULRInteractableComponent* Active = ActiveInteractable.Get();
	ALRPlayerCharacter* Player = GetOwnerPlayer();

	if (!Active || !Player || Active != FocusedInteractable.Get() || !Active->CanInteract(Player))
	{
		EndInteract();
		return;
	}

	HoldElapsed += DeltaTime;
	if (HoldElapsed >= FMath::Max(Active->HoldDuration, KINDA_SMALL_NUMBER))
	{
		ULRInteractableComponent* Completed = Active;
		EndInteract();
		Completed->CompleteInteract(Player);
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

	ActiveInteractable = Target;
	HoldElapsed = 0.f;

	if (Target->HoldDuration <= 0.f)
	{
		EndInteract();
		Target->CompleteInteract(Player);
		return;
	}

	bHolding = true;
}

void ULRInteractionComponent::EndInteract()
{
	bHolding = false;
	ActiveInteractable = nullptr;
	HoldElapsed = 0.f;
}

ULRInteractableComponent* ULRInteractionComponent::GetFocusedInteractable() const
{
	return FocusedInteractable.Get();
}

float ULRInteractionComponent::GetHoldProgress() const
{
	const ULRInteractableComponent* Active = ActiveInteractable.Get();
	if (!bHolding || !Active || Active->HoldDuration <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(HoldElapsed / Active->HoldDuration, 0.f, 1.f);
}

ALRPlayerCharacter* ULRInteractionComponent::GetOwnerPlayer() const
{
	return Cast<ALRPlayerCharacter>(GetOwner());
}

void ULRInteractionComponent::UpdateFocus()
{
	ULRInteractableComponent* NewFocus = nullptr;
	FHitResult Hit;

	if (TraceForInteractable(Hit))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			NewFocus = HitActor->FindComponentByClass<ULRInteractableComponent>();
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
