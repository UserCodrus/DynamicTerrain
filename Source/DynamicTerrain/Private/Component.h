#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "HAL/Runnable.h"

#include "Terrain.h"

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