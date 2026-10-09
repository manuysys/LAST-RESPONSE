#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LRGameMode.generated.h"

class ALRMissionDirector;

UCLASS()
class LASTRESPONSE_API ALRGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALRGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Mission")
	ALRMissionDirector* GetMissionDirector() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mission")
	bool bAutoStartMission = true;
};
