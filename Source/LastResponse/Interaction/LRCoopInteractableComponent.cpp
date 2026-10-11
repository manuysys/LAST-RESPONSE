#include "Interaction/LRCoopInteractableComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/LRPlayerCharacter.h"
#include "Player/LRInteractionComponent.h"

ULRCoopInteractableComponent::ULRCoopInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULRCoopInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULRCoopInteractableComponent, HolderCount);
	DOREPLIFETIME(ULRCoopInteractableComponent, CoopProgress);
}

void ULRCoopInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !bEnabled)
	{
		return;
	}

	PruneHolders();
	HolderCount = Holders.Num();

	if (HolderCount >= RequiredHolders)
	{
		CoopProgress += DeltaTime;
	}
	else if (CoopProgress > 0.f)
	{
		CoopProgress = FMath::Max(CoopProgress - DeltaTime * 2.f, 0.f);
	}

	if (CoopProgress >= FMath::Max(HoldDuration, KINDA_SMALL_NUMBER) && Holders.Num() > 0)
	{
		ALRPlayerCharacter* FirstHolder = Holders[0].Get();
		Holders.Reset();
		HolderCount = 0;
		CoopProgress = 0.f;
		bEnabled = false;
		CompleteInteract(FirstHolder);
	}
}

float ULRCoopInteractableComponent::GetCoopProgress() const
{
	if (HoldDuration <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(CoopProgress / HoldDuration, 0.f, 1.f);
}

void ULRCoopInteractableComponent::ServerAddHolder(ALRPlayerCharacter* Player)
{
	if (!Player || !bEnabled)
	{
		return;
	}

	if (!Holders.Contains(Player))
	{
		Holders.Add(Player);
		HolderCount = Holders.Num();
	}
}

void ULRCoopInteractableComponent::ServerRemoveHolder(ALRPlayerCharacter* Player)
{
	if (Holders.Remove(Player) > 0)
	{
		HolderCount = Holders.Num();
	}
}

void ULRCoopInteractableComponent::OnRep_HolderCount()
{
	OnHolderCountChanged.Broadcast(HolderCount, RequiredHolders);
}

void ULRCoopInteractableComponent::OnRep_CoopProgress()
{
}

void ULRCoopInteractableComponent::PruneHolders()
{
	for (int32 i = Holders.Num() - 1; i >= 0; --i)
	{
		if (!IsHolderActive(Holders[i].Get()))
		{
			Holders.RemoveAt(i);
		}
	}
}

bool ULRCoopInteractableComponent::IsHolderActive(ALRPlayerCharacter* Player) const
{
	if (!Player || Player->IsDowned())
	{
		return false;
	}

	const ULRInteractionComponent* Interaction = Player->GetInteractionComponent();
	return Interaction && Interaction->GetServerActiveTarget() == this;
}
