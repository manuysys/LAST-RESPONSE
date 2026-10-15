#include "World/LRCurrentZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Prototype/LRPrototypeVisuals.h"
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

	ZoneVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneVisual"));
	ZoneVisual->SetupAttachment(Zone);
	ZoneVisual->SetRelativeLocation(FVector(0.f, 0.f, -195.f));
	ZoneVisual->SetRelativeScale3D(FVector(8.f, 8.f, 0.1f));
	ZoneVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cube = FLRProto::CubeMesh())
	{
		ZoneVisual->SetStaticMesh(Cube);
	}
}

void ALRCurrentZone::BeginPlay()
{
	Super::BeginPlay();

	if (!FLRProto::UpgradeToRealModel(ZoneVisual, TEXT("SM_CurrentPad"), FVector::OneVector, FVector(0.f, 0.f, -200.f)))
	{
		FLRProto::ApplyColor(ZoneVisual, FLinearColor(0.1f, 0.55f, 0.9f));
	}
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
