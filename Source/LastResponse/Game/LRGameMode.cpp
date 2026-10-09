#include "Game/LRGameMode.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Game/LRMissionDirector.h"
#include "Player/LRPlayerCharacter.h"

ALRGameMode::ALRGameMode()
{
	DefaultPawnClass = ALRPlayerCharacter::StaticClass();
}

void ALRGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStartMission)
	{
		if (ALRMissionDirector* Director = GetMissionDirector())
		{
			Director->StartMission();
		}
	}
}

ALRMissionDirector* ALRGameMode::GetMissionDirector() const
{
	for (TActorIterator<ALRMissionDirector> It(GetWorld()); It; ++It)
	{
		return *It;
	}

	return nullptr;
}
