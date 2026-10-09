#include "World/LRCurrentZone.h"
#include "Components/BoxComponent.h"
#include "Player/LRPlayerCharacter.h"

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
	for (AActor* Actor : OverlappingActors)
	{
		ALRPlayerCharacter* Player = Cast<ALRPlayerCharacter>(Actor);
		if (!Player)
		{
			continue;
		}

		const float Multiplier = Player->HasRopeAssist() ? RopeAssistMultiplier : 1.f;
		Player->AddActorWorldOffset(Direction * PushSpeed * Multiplier * DeltaSeconds, true);
	}
}
