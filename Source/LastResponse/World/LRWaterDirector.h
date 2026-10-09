#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRWaterDirector.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLRWaterLevelChangedEvent, float, NewLevel);

UCLASS()
class LASTRESPONSE_API ALRWaterDirector : public AActor
{
	GENERATED_BODY()

public:
	ALRWaterDirector();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Water")
	void StartRise();

	UFUNCTION(BlueprintCallable, Category = "Water")
	void StopRise();

	UFUNCTION(BlueprintCallable, Category = "Water")
	void SetWaterLevel(float NewLevel);

	UFUNCTION(BlueprintPure, Category = "Water")
	float GetWaterLevel() const { return WaterLevel; }

	UPROPERTY(BlueprintAssignable, Category = "Water")
	FLRWaterLevelChangedEvent OnWaterLevelChanged;

protected:
	virtual void BeginPlay() override;

	void ApplyImmersion();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WaterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> FloodZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float WaterLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float RiseSpeed = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	float MaxWaterLevel = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	bool bRising = false;

private:
	void UpdateWaterVisual();

	TArray<TWeakObjectPtr<ACharacter>> ImmersedCharacters;
	TMap<TWeakObjectPtr<ACharacter>, float> ImmersedBaseSpeeds;
};
