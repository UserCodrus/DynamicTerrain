#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "HAL/Runnable.h"

#include "TerrainComponent.generated.h"

class ATerrain;

USTRUCT()
struct FTerrainVertex
{
	GENERATED_BODY()

	UPROPERTY()
		FVector Position;
	UPROPERTY()
		FVector Normal;
	UPROPERTY()
		FVector Tangent;
	UPROPERTY()
		FVector2D UV;

	FTerrainVertex() : Position(0.0f, 0.0f, 0.0f), Normal(0.0f, 0.0f, 1.0f), Tangent(1.0f, 0.0f, 0.0f), UV(0.0f, 0.0f) {}
};

class ComponentBuilder : public FRunnable
{
public:
	ComponentBuilder(const ATerrain* ParentTerrain);
	~ComponentBuilder();

	/// FRunnable Interface ///

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

	/// Builder Interface ///

	bool IsIdle();
	void Build(int32 component_x, int32 component_y);

	ComponentData* GetData();
	int32 GetSection();

	FRunnableThread* Thread = nullptr;				// The thread this interface runs on
	FThreadSafeCounter Counter;						// Thread counter for managing the thread
	ComponentData Data;								// The container for the data

private:
	const ATerrain* Terrain;						// The parent terrain actor

	int32 ComponentX;
	int32 ComponentY;

	bool Idle;
};

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
	// Set the size of the component
	void SetSize(uint32 NewSize);
	// Change the location of the component
	void SetOffset(int32 X, int32 Y);
	// Generate vertices for the component from a terrain
	void GenerateVertices(ATerrain* Terrain);

private:
	// The vertices for the mesh and collision
	UPROPERTY(VisibleAnywhere)
		TArray<FTerrainVertex> VertexBuffer;
	// The mesh indices
	UPROPERTY(VisibleAnywhere)
		TArray<uint32> IndexBuffer;
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

	friend class FTerrainComponentSceneProxy;
};