#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"

#include "DynamicMeshBuilder.h"

class UTerrainComponent;
struct FMapSection;

// A rendering proxy which stores rendering data for a single terrain component
// Functions for the proxy should only be called on the rendering thread (with the exception of the constructor)
// Use functions in UTerrainComponent to change proxies on the game thread
class FTerrainComponentSceneProxy : public FPrimitiveSceneProxy
{
public:
	FTerrainComponentSceneProxy(UTerrainComponent* Component);
	virtual ~FTerrainComponentSceneProxy();

	/// Scene Proxy Interface ///

	SIZE_T GetTypeHash() const override
	{
		static size_t unique_pointer;
		return reinterpret_cast<size_t>(&unique_pointer);
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return (sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize() const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual void GetDynamicMeshElements(const TArray< const FSceneView* >& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	/// Proxy Update Functions ///

	// Update rending data using the provided proxy
	void UpdateMap(TSharedPtr<FMapSection, ESPMode::ThreadSafe> SectionProxy);
	// Update UV tiling
	void UpdateUVs(int32 XOffset, int32 YOffset, float Tiling);

protected:
	// Initialize vertex buffers
	void Initialize(int32 X, int32 Y, float Tiling);
	// Update rendering data using the current map proxy data
	void UpdateMapData();
	// Update mesh UVs using the provided offsets and tiling
	void UpdateUVData(int32 XOffset, int32 YOffset, float Tiling);
	// Create a set of triangles
	void CreateIndexBuffer(TArray<uint32>& Indices, uint32 Stride);
	// Set LOD scales for each lod
	void ScaleLODs(float Scale);

	// The heightmap data the component needs to render
	TSharedPtr<FMapSection, ESPMode::ThreadSafe> MapProxy = nullptr;
	// The width of the component, the number of vertices is Size * Size + 1
	uint32 Size;

	// The vertex buffers containing mesh data
	FStaticMeshVertexBuffers VertexBuffers;
	// The triangles used by the component's mesh
	TArray<FDynamicMeshIndexBuffer32> IndexBuffers;
	// The vertex factory for storing vertex type data
	FLocalVertexFactory VertexFactory;

	// The material used to render the component
	UMaterialInterface* Material;
	FMaterialRelevance MaterialRelevance;

	// The number of LODs to create for the mesh
	uint32 MaxLOD;
	// LOD scales for each individual LOD
	TArray<float> LODScales;
};