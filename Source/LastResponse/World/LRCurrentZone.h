#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRCurrentZone.generated.h"

class UBoxComponent;

UCLASS()
class LASTRESPONSE_API ALRCurrentZone : public AActor
{
	GENERATED_BODY()

public:
	ALRCurrentZone();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Current")
	void SetActive(bool bNewActive) { bActive = bNewActive; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Zone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current")
	FVector PushDirection = FVector(1.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current")
	float PushSpeed = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RopeAssistMultiplier = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current")
	bool bActive = true;
};
