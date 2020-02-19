#pragma once

#include "Terrain.h"

#include "EdMode.h"
#include "EditorModeRegistry.h"

class FDynamicTerrainMode : public FEdMode
{
public:
	/// Engine Functions ///

	virtual void Enter() override;

	virtual void Exit() override;

	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

	virtual bool DisallowMouseDeltaTracking() const override;

	virtual bool HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;

	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;

	const static FEditorModeID DynamicTerrainModeID;

protected:
	ATerrain* Terrain = nullptr;
};