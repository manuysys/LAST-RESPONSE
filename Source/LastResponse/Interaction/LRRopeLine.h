#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LRRopeLine.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ALRRopeAnchor;

UCLASS()
class LASTRESPONSE_API ALRRopeLine : public AActor
{
	GENERATED_BODY()

public:
	ALRRopeLine();

	void Initialize(ALRRopeAnchor* AnchorA, ALRRopeAnchor* AnchorB);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Corridor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RopeMesh;
};
