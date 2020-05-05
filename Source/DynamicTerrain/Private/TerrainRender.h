#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"

#include "DynamicMeshBuilder.h"

class UTerrainComponent;
struct FMapSection;

struct FTerrainTangents
{
	FPackedNormal Tangent;
	FPackedNormal Normal;
};

class FTerrainVertexBuffer : public FVertexBuffer
{
public:
	FTerrainVertexBuffer(int32 DataSize, uint32 BufferStride, uint8 BufferFormat);

	// Initialize the buffer
	virtual void InitRHI() override;
	// Delete the buffer
	virtual void ReleaseRHI() override;

	// Bind the buffer to vertex factory data
	virtual void Bind(FLocalVertexFactory::FDataType& DataType) = 0;

	// The size of the component stored in the buffer
	uint32 ComponentSize;

protected:
	// Whether the buffer will be used for static or dynamic rendering (currently only supports dynamic)
	const EBufferUsageFlags StaticFlag;
	// The shader resource view for the buffer data
	FShaderResourceViewRHIRef ShaderResourceView;

	// The size of each piece of vertex data in bytes
	const uint32 Size;
	// The stride of the buffer data
	const uint32 Stride;
	// The format of the buffer data
	const uint8 Format;
};

class FTerrainPositionBuffer : public FTerrainVertexBuffer
{
public:
	FTerrainPositionBuffer();

	void FillBuffer();
	void UpdateBuffer(TSharedPtr<FMapSection, ESPMode::ThreadSafe> Map);

	virtual void Bind(FLocalVertexFactory::FDataType& DataType);

	TArray<FVector> Data;
};

class FTerrainUVBuffer : public FTerrainVertexBuffer
{
public:
	FTerrainUVBuffer();

	void FillBuffer(int32 XOffset, int32 YOffset, float Tiling);

	virtual void Bind(FLocalVertexFactory::FDataType& DataType);
};

class FTerrainTangentBuffer : public FTerrainVertexBuffer
{
public:
	FTerrainTangentBuffer();

	//void Set();
	void UpdateBuffer(TSharedPtr<FMapSection, ESPMode::ThreadSafe> Map);

	virtual void Bind(FLocalVertexFactory::FDataType& DataType);
};

// A rendering proxy which stores rendering data for a single terrain component
// All proxy functions are only callable on the render thread, with the exception of the constructor
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

	void FillBuffers(int32 X, int32 Y, float Tiling);
	void BindData();

	/// Proxy Update Functions ///

	// Change the scaling of the UV data
	void SetTiling(float Value);

protected:
	// The render data for the terrain object
	TSharedPtr<FMapSection, ESPMode::ThreadSafe> MapProxy = nullptr;

	// Vertex position data
	FTerrainPositionBuffer PositionBuffer;
	// Vertex tanget and normal vectors
	FTerrainTangentBuffer TangentBuffer;
	// Vertex UVs
	FTerrainUVBuffer UVBuffer;

	// The triangles used by the component
	FDynamicMeshIndexBuffer32 IndexBuffer;
	// The vertex factory for storing vertex data
	FLocalVertexFactory VertexFactory;

	// The material used to render the component
	UMaterialInterface* Material;
	FMaterialRelevance MaterialRelevance;
};