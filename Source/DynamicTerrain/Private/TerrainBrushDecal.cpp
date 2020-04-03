#include "TerrainBrushDecal.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UBrushDecal::UBrushDecal()
{
	SetAbsolute(true, true, true);
	SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	// Get the brush material
	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/DynamicTerrain/Materials/M_BrushDecal.M_BrushDecal'"));
	if (Material.Succeeded())
	{
		BrushMaterial = Material.Object;
	}
}

void UBrushDecal::OnComponentCreated()
{
	// Set the material for the decal
	SetDecalMaterial(BrushMaterial);
	BrushInstance = CreateDynamicMaterialInstance();
}

void UBrushDecal::Resize(FTerrainTool* Tool, ATerrain* Terrain)
{
	// Scale the brush size based on the terrain's scale
	float Radius = Terrain->GetActorScale3D().X * Tool->Size;
	float Falloff = Terrain->GetActorScale3D().X * Tool->Falloff;

	float width = Radius + Falloff + 100.0f;
	DecalSize = FVector(width, width, Terrain->GetActorScale3D().Z * 200.0f);

	BrushInstance->SetScalarParameterValue(TEXT("Radius"), Radius);
	BrushInstance->SetScalarParameterValue(TEXT("Falloff"), Falloff);
}

void UBrushDecal::ChangeColor(FColor Color)
{
	BrushInstance->SetVectorParameterValue(TEXT("Color"), Color);
}