#include "Prototype/LRPrototypeVisuals.h"

#include "Components/MeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"

namespace
{
	UStaticMesh* LoadBasicShape(const TCHAR* ShapeName)
	{
		return LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"), ShapeName, ShapeName));
	}

	UMaterialInterface* LoadPrototypeMaterial(const TCHAR* MaterialName)
	{
		const FString PackageName = FString::Printf(TEXT("/Game/Prototype/Materials/%s"), MaterialName);
		if (!FPackageName::DoesPackageExist(PackageName))
		{
			return nullptr;
		}

		return LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("%s.%s"), *PackageName, MaterialName));
	}

	UMaterialInterface* ResolveColorMaterial(const TCHAR* PreferredMaterialName)
	{
		if (PreferredMaterialName)
		{
			if (UMaterialInterface* Preferred = LoadPrototypeMaterial(PreferredMaterialName))
			{
				return Preferred;
			}
		}

		if (UMaterialInterface* ColorMaterial = LoadPrototypeMaterial(TEXT("M_PrototypeColor")))
		{
			return ColorMaterial;
		}

		return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
}

UStaticMesh* FLRProto::CubeMesh()
{
	return LoadBasicShape(TEXT("Cube"));
}

UStaticMesh* FLRProto::CylinderMesh()
{
	return LoadBasicShape(TEXT("Cylinder"));
}

UStaticMesh* FLRProto::SphereMesh()
{
	return LoadBasicShape(TEXT("Sphere"));
}

UStaticMesh* FLRProto::PlaneMesh()
{
	return LoadBasicShape(TEXT("Plane"));
}

UStaticMesh* FLRProto::ConeMesh()
{
	return LoadBasicShape(TEXT("Cone"));
}

UMaterialInterface* FLRProto::ColorMaterial()
{
	return ResolveColorMaterial(nullptr);
}

UMaterialInterface* FLRProto::WaterMaterial()
{
	return ResolveColorMaterial(TEXT("M_Water"));
}

void FLRProto::ApplyColor(UMeshComponent* Mesh, const FLinearColor& Color)
{
	if (!Mesh)
	{
		return;
	}

	if (UMaterialInterface* Material = ColorMaterial())
	{
		if (UMaterialInstanceDynamic* DynamicMaterial = Mesh->CreateDynamicMaterialInstance(0, Material))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		}
	}
}

void FLRProto::ApplyWater(UMeshComponent* Mesh, const FLinearColor& Color, float Opacity)
{
	if (!Mesh)
	{
		return;
	}

	if (UMaterialInterface* Material = WaterMaterial())
	{
		if (UMaterialInstanceDynamic* DynamicMaterial = Mesh->CreateDynamicMaterialInstance(0, Material))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
			DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		}
	}
}
