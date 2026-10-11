#include "World/LRCurrentZone.h"
#include "Components/BoxComponent.h"
#include "Player/LRPlayerCharacter.h"
#include "World/LRStormDirector.h"

ALRCurrentZone::ALRCurrentZone()
{
	PrimaryActorTick.bCanEverTick = true;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	SetRootComponent(Zone);
	Zone->SetBoxExtent(FVector(400.f, 400.f, 200.f));
	Zone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Zone->SetCollisionResponseToAllChannels(ECR_Ignore);
	Zone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Zone->SetGenerateOverlapEvents(true);
}

void ALRCurrentZone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bActive || !Zone)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	Zone->GetOverlappingActors(OverlappingActors, ALRPlayerCharacter::StaticClass());

	const FVector Direction = PushDirection.GetSafeNormal();

	float WindMultiplier = 1.f;
	if (const ALRStormDirector* Storm = ALRStormDirector::FindStorm(GetWorld()))
	{
		WindMultiplier = Storm->GetWindMultiplier();
	}

	for (AActor* Actor : OverlappingActors)
	{
		ALRPlayerCharacter* Player = Cast<ALRPlayerCharacter>(Actor);
		if (!Player)
		{
			continue;
		}

		const float Multiplier = Player->HasRopeAssist() ? RopeAssistMultiplier : 1.f;
		Player->AddActorWorldOffset(Direction * PushSpeed * WindMultiplier * Multiplier * DeltaSeconds, true);
	}
}
