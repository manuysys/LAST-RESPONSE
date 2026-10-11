#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRStormDirector.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLRStormUpdatedEvent, float, RainIntensity, float, WindStrength);

UCLASS()
class LASTRESPONSE_API ALRStormDirector : public AActor
{
	GENERATED_BODY()

public:
	ALRStormDirector();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Storm")
	float GetRainIntensity() const { return RainIntensity; }

	UFUNCTION(BlueprintPure, Category = "Storm")
	float GetWindStrength() const { return WindStrength; }

	UFUNCTION(BlueprintPure, Category = "Storm")
	float GetWindMultiplier() const { return 1.f + WindStrength * 0.5f; }

	UFUNCTION(BlueprintCallable, Category = "Storm")
	void SetStorm(float NewRain, float NewWind);

	UPROPERTY(BlueprintAssignable, Category = "Storm")
	FLRStormUpdatedEvent OnStormUpdated;

	UFUNCTION(BlueprintImplementableEvent, Category = "Storm")
	void BP_OnStormUpdated(float NewRain, float NewWind);

	UFUNCTION(BlueprintImplementableEvent, Category = "Storm")
	void BP_OnLightning();

	static ALRStormDirector* FindStorm(UWorld* World);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLightning();

	UFUNCTION()
	void OnRep_StormParams();

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_StormParams, BlueprintReadOnly, Category = "Storm", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RainIntensity = 0.6f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_StormParams, BlueprintReadOnly, Category = "Storm", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindStrength = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
	float LightningMinInterval = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storm")
	float LightningMaxInterval = 20.f;

private:
	float LightningTimer = 0.f;
};
