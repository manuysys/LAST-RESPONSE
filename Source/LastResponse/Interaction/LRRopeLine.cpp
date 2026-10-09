#include "Interaction/LRRopeLine.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/LRRopeAnchor.h"
#include "Player/LRPlayerCharacter.h"

ALRRopeLine::ALRRopeLine()
{
	PrimaryActorTick.bCanEverTick = false;

	Corridor = CreateDefaultSubobject<UBoxComponent>(TEXT("Corridor"));
	SetRootComponent(Corridor);
	Corridor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Corridor->SetCollisionResponseToAllChannels(ECR_Ignore);
	Corridor->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Corridor->SetGenerateOverlapEvents(true);

	RopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RopeMesh"));
	RopeMesh->SetupAttachment(Corridor);
	RopeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RopeMesh->SetMobility(EComponentMobility::Movable);
}

void ALRRopeLine::BeginPlay()
{
	Super::BeginPlay();

	if (Corridor)
	{
		Corridor->OnComponentBeginOverlap.AddDynamic(this, &ALRRopeLine::HandleBeginOverlap);
		Corridor->OnComponentEndOverlap.AddDynamic(this, &ALRRopeLine::HandleEndOverlap);
	}
}

void ALRRopeLine::Initialize(ALRRopeAnchor* AnchorA, ALRRopeAnchor* AnchorB)
{
	if (!AnchorA || !AnchorB)
	{
		return;
	}

	const FVector A = AnchorA->GetActorLocation();
	const FVector B = AnchorB->GetActorLocation();
	const FVector Direction = B - A;
	const float Length = Direction.Size();

	SetActorLocation((A + B) * 0.5f);
	SetActorRotation(FRotator(0.f, Direction.Rotation().Yaw, 0.f));

	if (Corridor)
	{
		Corridor->SetBoxExtent(FVector(FMath::Max(Length * 0.5f, 10.f), 120.f, 150.f));
	}

	if (RopeMesh)
	{
		RopeMesh->SetRelativeScale3D(FVector(FMath::Max(Length / 100.f, 0.1f), 1.f, 1.f));
	}
}

void ALRRopeLine::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ALRPlayerCharacter* Player = Cast<ALRPlayerCharacter>(OtherActor))
	{
		Player->AddRopeAssist();
	}
}

void ALRRopeLine::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ALRPlayerCharacter* Player = Cast<ALRPlayerCharacter>(OtherActor))
	{
		Player->RemoveRopeAssist();
	}
}
