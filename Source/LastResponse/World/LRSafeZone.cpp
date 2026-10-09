#include "World/LRSafeZone.h"
#include "Components/BoxComponent.h"
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
}

void ALRSafeZone::BeginPlay()
{
	Super::BeginPlay();

	if (Zone)
	{
		Zone->OnComponentBeginOverlap.AddDynamic(this, &ALRSafeZone::HandleBeginOverlap);
	}
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
