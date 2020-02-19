#include "DynamicTerrainMode.h"

#include "EngineUtils.h"
#include "EditorViewportClient.h"

const FEditorModeID FDynamicTerrainMode::DynamicTerrainModeID = TEXT("DynamicTerrainModeID");

void FDynamicTerrainMode::Enter()
{
	FEdMode::Enter();

	for (TActorIterator<ATerrain> itr(GetWorld()); itr; ++itr)
	{
		Terrain = *itr;
		Terrain->ShowBrush(true);
		break;
	}

	SelectNone();
}

void FDynamicTerrainMode::Exit()
{
	if (Terrain != nullptr)
	{
		Terrain->ShowBrush(false);
	}

	FEdMode::Exit();
}

void FDynamicTerrainMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	//ViewportClient->Viewport->CaptureMouse(true);

	// Get the scene view
	FSceneViewFamilyContext context(FSceneViewFamily::ConstructionValues(ViewportClient->Viewport, ViewportClient->GetScene(), ViewportClient->EngineShowFlags));
	FSceneView* View = ViewportClient->CalcSceneView(&context);

	// Get the mouse location
	FVector2D Mouse(ViewportClient->Viewport->GetMouseX(), ViewportClient->Viewport->GetMouseY());

	// Project the mouse into the world
	FVector WorldOrigin;
	FVector WorldDirection;

	FSceneView::DeprojectScreenToWorld(Mouse, View->UnconstrainedViewRect, View->ViewMatrices.GetInvViewProjectionMatrix(), WorldOrigin, WorldDirection);

	// Trace from the viewport outward under the cursor
	FHitResult hit;
	ViewportClient->GetWorld()->LineTraceSingleByChannel(hit, WorldOrigin, WorldOrigin + WorldDirection * 50000.0f, ECollisionChannel::ECC_WorldDynamic);

	// Position the brush and apply it if the tool is active
	if (Terrain != nullptr)
	{
		Terrain->SetBrushPosition(hit.Location);
	}
}

bool FDynamicTerrainMode::DisallowMouseDeltaTracking() const
{
	return false;
}

bool FDynamicTerrainMode::HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	return true;
}

bool FDynamicTerrainMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == EInputEvent::IE_Pressed)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, "Click");
			Viewport->CaptureMouse(true);
		}
		else if (Event == EInputEvent::IE_Released)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, "Unclick");
			Viewport->CaptureMouse(false);
		}

		return true;
	}

	return false;
}