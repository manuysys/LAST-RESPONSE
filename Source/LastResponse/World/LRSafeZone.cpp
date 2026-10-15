#include "World/LRSafeZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Victim/LRVictimCharacter.h"

ALRSafeZone::ALRSafeZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	SetRootComponent(Zone);
	Zone->SetBoxExtent(FVector(300.f, 300.f, 200.f));
	Zone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Zone->SetCollisionResponseToAllChannels(ECR_Ignore);
	Zone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Zone->SetGenerateOverlapEvents(true);

	ZoneVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneVisual"));
	ZoneVisual->SetupAttachment(Zone);
	ZoneVisual->SetRelativeLocation(FVector(0.f, 0.f, -195.f));
	ZoneVisual->SetRelativeScale3D(FVector(6.f, 6.f, 0.1f));
	ZoneVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cube = FLRProto::CubeMesh())
	{
		ZoneVisual->SetStaticMesh(Cube);
	}
}

void ALRSafeZone::BeginPlay()
{
	Super::BeginPlay();

	if (Zone)
	{
		Zone->OnComponentBeginOverlap.AddDynamic(this, &ALRSafeZone::HandleBeginOverlap);
	}

	FLRProto::ApplyColor(ZoneVisual, FLinearColor(0.05f, 0.85f, 0.25f));
}

void ALRSafeZone::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	ALRVictimCharacter* Victim = Cast<ALRVictimCharacter>(OtherActor);
	if (Victim && Victim->GetVictimState() == ELRVictimState::Assisted)
	{
		Victim->SetVictimState(ELRVictimState::Evacuated);
	}
}
