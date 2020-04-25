#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"

#include "DynamicMeshBuilder.h"

class UTerrainComponent;

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

	// Reallocate the buffer
	void Reset(uint32 VertexCount);

	// Bind the buffer to vertex factory data
	virtual void Bind(FLocalVertexFactory::FDataType& DataType) = 0;

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

	// The number of vertices
	uint32 Vertices;
};

class FTerrainPositionBuffer : public FTerrainVertexBuffer
{
public:
	FTerrainPositionBuffer();

	void Set();

	virtual void Bind(FLocalVertexFactory::FDataType& DataType);

	TArray<FVector> Data;
};

class FTerrainUVBuffer : public FTerrainVertexBuffer
{
public:
	FTerrainUVBuffer();

	void Set();

	virtual void Bind(FLocalVertexFactory::FDataType& DataType);

	TArray<FVector2D> Data;
};

class FTerrainTangentBuffer : public FTerrainVertexBuffer
{
public:
	FTerrainTangentBuffer();

	void Set();

	virtual void Bind(FLocalVertexFactory::FDataType& DataType);

	TArray<FTerrainTangents> Data;
};

class FTerrainColorBuffer : public FTerrainVertexBuffer
{
public:
	FTerrainColorBuffer();

	void Set();

	virtual void Bind(FLocalVertexFactory::FDataType& DataType);

	TArray<FColor> Data;
};

class FTerrainComponentSceneProxy : public FPrimitiveSceneProxy
{
public:
	FTerrainComponentSceneProxy(UTerrainComponent* Component);
	virtual ~FTerrainComponentSceneProxy();

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

	void SizeBuffers();
	void FillBuffers();
	void BindData();

protected:
	FTerrainPositionBuffer PositionBuffer;
	FTerrainUVBuffer UVBuffer;
	FTerrainTangentBuffer TangentBuffer;
	FTerrainColorBuffer ColorBuffer;

	//FStaticMeshVertexBuffers VertexBuffers;
	FDynamicMeshIndexBuffer32 IndexBuffer;
	FLocalVertexFactory VertexFactory;

	UMaterialInterface* Material;
	FMaterialRelevance MaterialRelevance;
};