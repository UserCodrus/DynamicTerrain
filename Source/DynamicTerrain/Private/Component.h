#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "HAL/Runnable.h"

#include "Terrain.h"

struct ComponentData
{
	void allocate(uint16 size, uint16 section_id);

	uint16 section;
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> UV0;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
};

class ComponentBuilder : public FRunnable
{
public:
	ComponentBuilder(const UHeightMap* Map, const TArray<FVector>* Normals, const TArray<FProcMeshTangent>* Tangents, float map_height, int32 component_size);
	~ComponentBuilder();

	/// FRunnable Interface ///

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

	/// Builder Interface ///

	bool IsIdle();
	void Build(int32 _component_x, int32 _component_y, uint16 section_number);
	ComponentData* GetData();

	FRunnableThread* thread = nullptr;				// The thread this interface runs on
	FThreadSafeCounter counter;						// Thread counter for managing the thread
	ComponentData data;								// The container for the data

private:
	const UHeightMap* heightmap;					// The heightmap to draw data from
	const TArray<FVector>* normalmap;				// The vectormap to draw normal data from
	const TArray<FProcMeshTangent>* tangentmap;		// The vectormap to draw tangent data from

	float total_height;								// The Heigth of the parent terrain
	int32 component_width;							// The ComponentSize of the parent terrain

	int32 component_x;
	int32 component_y;

	bool idle;
};