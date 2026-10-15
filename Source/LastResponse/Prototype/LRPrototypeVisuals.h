#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UMeshComponent;
class UStaticMesh;

struct LASTRESPONSE_API FLRProto
{
	static UStaticMesh* CubeMesh();
	static UStaticMesh* CylinderMesh();
	static UStaticMesh* SphereMesh();
	static UStaticMesh* PlaneMesh();
	static UStaticMesh* ConeMesh();

	static UMaterialInterface* ColorMaterial();
	static UMaterialInterface* WaterMaterial();

	static void ApplyColor(UMeshComponent* Mesh, const FLinearColor& Color);
	static void ApplyWater(UMeshComponent* Mesh, const FLinearColor& Color, float Opacity);
};
