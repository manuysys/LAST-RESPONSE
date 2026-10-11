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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	UPROPERTY(ReplicatedUsing = OnRep_Endpoints)
	FVector_NetQuantize EndpointA = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_Endpoints)
	FVector_NetQuantize EndpointB = FVector::ZeroVector;

	UFUNCTION()
	void OnRep_Endpoints();

	void RebuildFromEndpoints();
};
