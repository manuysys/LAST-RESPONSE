#include "Debug/LRPlaytestRecorder.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void ULRPlaytestRecorder::BeginSession(const FString& MapName)
{
	Lines.Reset();
	SessionMap = MapName;
	SessionTimestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	SessionStartTime = FPlatformTime::Seconds();
	bSessionActive = true;
	RecordEvent(TEXT("Session"), FString::Printf(TEXT("Begin %s"), *MapName));
}

void ULRPlaytestRecorder::RecordEvent(const FString& Category, const FString& Detail)
{
	if (!bSessionActive)
	{
		return;
	}

	const double Elapsed = FPlatformTime::Seconds() - SessionStartTime;
	Lines.Add(FString::Printf(TEXT("%.2f,%s,%s"), Elapsed, *Category, *Detail));
}

void ULRPlaytestRecorder::EndSession(const FString& Summary)
{
	if (!bSessionActive)
	{
		return;
	}

	RecordEvent(TEXT("Session"), FString::Printf(TEXT("End %s"), *Summary));
	bSessionActive = false;

	const FString FilePath = BuildFilePath();
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(FilePath), true);
	FFileHelper::SaveStringArrayToFile(Lines, *FilePath);
}

FString ULRPlaytestRecorder::BuildFilePath() const
{
	const FString Directory = FPaths::ProjectSavedDir() / TEXT("Playtest");
	return Directory / FString::Printf(TEXT("lr_%s_%s.csv"), *SessionMap, *SessionTimestamp);
}
