#include "World/LRWaterDirector.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/LRPlayerCharacter.h"

ALRWaterDirector::ALRWaterDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	FloodZone = CreateDefaultSubobject<UBoxComponent>(TEXT("FloodZone"));
	SetRootComponent(FloodZone);
	FloodZone->SetBoxExtent(FVector(3000.f, 3000.f, 500.f));
	FloodZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FloodZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	FloodZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FloodZone->SetGenerateOverlapEvents(true);

	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
	WaterMesh->SetupAttachment(FloodZone);
	WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterMesh->SetMobility(EComponentMobility::Movable);
}

void ALRWaterDirector::BeginPlay()
{
	Super::BeginPlay();

	UpdateWaterVisual();
}

void ALRWaterDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bRising)
	{
		SetWaterLevel(FMath::Min(WaterLevel + RiseSpeed * DeltaSeconds, MaxWaterLevel));
	}

	ApplyImmersion();
}

void ALRWaterDirector::StartRise()
{
	bRising = true;
}

void ALRWaterDirector::StopRise()
{
	bRising = false;
}

void ALRWaterDirector::SetWaterLevel(float NewLevel)
{
	if (FMath::IsNearlyEqual(WaterLevel, NewLevel))
	{
		return;
	}

	WaterLevel = NewLevel;
	UpdateWaterVisual();
	OnWaterLevelChanged.Broadcast(WaterLevel);
}

void ALRWaterDirector::UpdateWaterVisual()
{
	if (WaterMesh)
	{
		WaterMesh->SetRelativeLocation(FVector(0.f, 0.f, WaterLevel - GetActorLocation().Z));
	}
}

void ALRWaterDirector::ApplyImmersion()
{
	if (!FloodZone)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	FloodZone->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	TArray<TWeakObjectPtr<ACharacter>> CurrentImmersed;

	for (AActor* Actor : OverlappingActors)
	{
		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character)
		{
			continue;
		}

		const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		if (!Capsule)
		{
			continue;
		}

		const float Height = Capsule->GetScaledCapsuleHalfHeight() * 2.f;
		const float FeetZ = Character->GetActorLocation().Z - Capsule->GetScaledCapsuleHalfHeight();
		const float Ratio = FMath::Clamp((WaterLevel - FeetZ) / FMath::Max(Height, 1.f), 0.f, 1.f);

		CurrentImmersed.Add(Character);

		if (ALRPlayerCharacter* Player = Cast<ALRPlayerCharacter>(Character))
		{
			Player->SetWaterImmersion(Ratio);
		}
		else if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			float& BaseSpeed = ImmersedBaseSpeeds.FindOrAdd(Character);
			if (BaseSpeed <= 0.f)
			{
				BaseSpeed = Movement->MaxWalkSpeed;
			}
			Movement->MaxWalkSpeed = BaseSpeed * FMath::Lerp(1.f, 0.35f, Ratio);
		}
	}

	for (const TWeakObjectPtr<ACharacter>& Previous : ImmersedCharacters)
	{
		if (!Previous.IsValid() || CurrentImmersed.Contains(Previous))
		{
			continue;
		}

		if (ALRPlayerCharacter* Player = Cast<ALRPlayerCharacter>(Previous.Get()))
		{
			Player->SetWaterImmersion(0.f);
		}
		else if (UCharacterMovementComponent* Movement = Previous->GetCharacterMovement())
		{
			if (const float* BaseSpeed = ImmersedBaseSpeeds.Find(Previous))
			{
				Movement->MaxWalkSpeed = *BaseSpeed;
			}
		}
	}

	ImmersedCharacters = CurrentImmersed;
}
