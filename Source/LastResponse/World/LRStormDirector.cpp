#include "World/LRStormDirector.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

ALRStormDirector::ALRStormDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALRStormDirector::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALRStormDirector, RainIntensity);
	DOREPLIFETIME(ALRStormDirector, WindStrength);
}

void ALRStormDirector::BeginPlay()
{
	Super::BeginPlay();

	LightningTimer = FMath::FRandRange(LightningMinInterval, LightningMaxInterval);
	OnRep_StormParams();
}

void ALRStormDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (RainIntensity < 0.5f)
	{
		return;
	}

	LightningTimer -= DeltaSeconds;
	if (LightningTimer <= 0.f)
	{
		LightningTimer = FMath::FRandRange(LightningMinInterval, LightningMaxInterval);
		MulticastLightning();
	}
}

void ALRStormDirector::SetStorm(float NewRain, float NewWind)
{
	if (!HasAuthority())
	{
		return;
	}

	RainIntensity = FMath::Clamp(NewRain, 0.f, 1.f);
	WindStrength = FMath::Clamp(NewWind, 0.f, 1.f);
	OnRep_StormParams();
}

ALRStormDirector* ALRStormDirector::FindStorm(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ALRStormDirector> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void ALRStormDirector::MulticastLightning_Implementation()
{
	BP_OnLightning();
}

void ALRStormDirector::OnRep_StormParams()
{
	OnStormUpdated.Broadcast(RainIntensity, WindStrength);
	BP_OnStormUpdated(RainIntensity, WindStrength);
}
