#pragma once

#include "TerrainHeightMap.h"

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"

#include "TerrainComponent.generated.h"

class ATerrain;

UCLASS(hidecategories = (Object, LOD, Physics, Collision), editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class DYNAMICTERRAIN_API UTerrainComponent : public UMeshComponent
{
	GENERATED_BODY()

	/// Mesh Component Interface ///

public:
	UTerrainComponent(const FObjectInitializer& ObjectInitializer);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override;

private:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	/// Terrain Interface ///

public:
	// Initialize the component
	void Initialize(ATerrain* Terrain, int32 X, int32 Y);
	// Initialize mesh data
	void CreateMeshData();

	// Set the size of the component
	void SetSize(uint32 NewSize);
	// Generate vertices for the component from a terrain
	void GenerateVertices(ATerrain* Terrain);

	// Update rendering data from a heightmap section
	void Update(TSharedPtr<FMapSection, ESPMode::ThreadSafe> NewSection);

	// Get the map data for this section
	TSharedPtr<FMapSection, ESPMode::ThreadSafe> GetMapProxy();

private:
	// Verify that the map proxy exists
	void VerifyMapProxy();

	// The mesh indices
	UPROPERTY(VisibleAnywhere)
		TArray<uint32> IndexBuffer;
	// The mesh vertices
	UPROPERTY(VisibleAnywhere)
		TArray<FVector> Vertices;

	// The size of the component
	UPROPERTY(VisibleAnywhere)
		uint32 Size;
	// The offset of the component on the X axis
	UPROPERTY(VisibleAnywhere)
		int32 XOffset;
	// The offset of the component on the Y axis
	UPROPERTY(VisibleAnywhere)
		int32 YOffset;
	// The UV Tiling of the component
	UPROPERTY(VisibleAnywhere)
		float Tiling;

	// The render data for the terrain component
	TSharedPtr<FMapSection, ESPMode::ThreadSafe> MapProxy;

	friend class FTerrainComponentSceneProxy;
};