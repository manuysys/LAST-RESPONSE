#include "Meta/LRMetaLibrary.h"

float ULRMetaLibrary::GetToolWeight(FName ToolId)
{
	static const TMap<FName, float> Weights = {
		{ TEXT("PryBar"), 3.f },
		{ TEXT("Rope"), 2.f },
		{ TEXT("Medkit"), 1.f },
		{ TEXT("Chainsaw"), 6.f },
		{ TEXT("Extinguisher"), 4.f },
		{ TEXT("Stretcher"), 5.f }
	};

	const float* Found = Weights.Find(ToolId);
	return Found ? *Found : 1.f;
}

int32 ULRMetaLibrary::GetToolPrice(FName ToolId)
{
	static const TMap<FName, int32> Prices = {
		{ TEXT("PryBar"), 150 },
		{ TEXT("Rope"), 100 },
		{ TEXT("Medkit"), 80 },
		{ TEXT("Chainsaw"), 400 },
		{ TEXT("Extinguisher"), 250 },
		{ TEXT("Stretcher"), 300 }
	};

	const int32* Found = Prices.Find(ToolId);
	return Found ? *Found : 100;
}
