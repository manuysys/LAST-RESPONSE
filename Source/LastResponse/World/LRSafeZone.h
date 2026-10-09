#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRSafeZone.generated.h"

class UBoxComponent;

UCLASS()
class LASTRESPONSE_API ALRSafeZone : public AActor
{
	GENERATED_BODY()

public:
	ALRSafeZone();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Zone;
};
