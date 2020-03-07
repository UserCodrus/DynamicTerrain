#include "DynamicTerrainMode.h"
#include "DynamicTerrainModeToolkit.h"

#include "EngineUtils.h"
#include "EditorViewportClient.h"

#include "EditorModeManager.h"

const FEditorModeID FDynamicTerrainMode::DynamicTerrainModeID = TEXT("DynamicTerrainModeID");

FDynamicTerrainMode::FDynamicTerrainMode()
{
	Settings = NewObject<UDynamicTerrainSettings>(GetTransientPackage(), TEXT("DynamicTerrainSettings"), RF_Transactional);

	// Create editor mode
	Modes.SetNum((int)TerrainModeID::NUM);
	Modes[(int)TerrainModeID::SCULPT] = new FDynamicTerrainToolMode("Sculpt", TerrainModeID::SCULPT);
	Modes[(int)TerrainModeID::MANAGE] = new FDynamicTerrainToolMode("Manage", TerrainModeID::MANAGE);
	Modes[(int)TerrainModeID::GENERATE] = new FDynamicTerrainToolMode("Generate", TerrainModeID::GENERATE);

	CurrentMode = Modes[0];
}

FDynamicTerrainMode::~FDynamicTerrainMode()
{
	for (int32 i = 0; i < Modes.Num(); ++i)
	{
		delete Modes[i];
	}
	Modes.Empty();
}

/// Engine Functions ///

void FDynamicTerrainMode::Enter()
{
	FEdMode::Enter();

	// Initialize the toolkit
	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FDynamicTerrainModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	// Select the first available terrain actor
	for (TActorIterator<ATerrain> itr(GetWorld()); itr; ++itr)
	{
		Terrain = *itr;
		Terrain->ShowBrush(true);
		break;
	}

	//SelectNone();
}

void FDynamicTerrainMode::Exit()
{
	// Disable the brush
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

	// Trace from the viewport outward under the mouse
	FHitResult hit;
	ViewportClient->GetWorld()->LineTraceSingleByChannel(hit, WorldOrigin, WorldOrigin + WorldDirection * 50000.0f, ECollisionChannel::ECC_WorldDynamic);

	// Position the brush and apply it if the tool is active
	if (Terrain != nullptr)
	{
		FTerrainTool* tool = Tools.GetTool();

		// Adjust the brush display
		Terrain->SetBrushPosition(hit.Location);
		Terrain->SetBrushSize(tool->Size, tool->Falloff);

		if (UseTool)
		{
			// Set properties of the current tool
			tool->Invert = InvertTool;

			// Apply the tool
			tool->Apply(Terrain, hit.Location, DeltaTime);

			// Apply changes to the terrain
			Terrain->Update();
		}
	}
}

bool FDynamicTerrainMode::DisallowMouseDeltaTracking() const
{
	return false;
}

bool FDynamicTerrainMode::HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	// Notify the engine that mouse clicks were handled to prevent actors from being selected when in terrain mode
	return true;
}

bool FDynamicTerrainMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	// Handle left clicks
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == EInputEvent::IE_Pressed)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, "Click");
			Viewport->CaptureMouse(true);

			UseTool = true;
		}
		else if (Event == EInputEvent::IE_Released)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, "Unclick");
			Viewport->CaptureMouse(false);

			UseTool = false;
		}

		return true;
	}

	// Handle modifier keys
	if (Key == EKeys::LeftShift)
	{
		if (Event == EInputEvent::IE_Pressed)
		{
			InvertTool = true;
		}
		else if (Event == EInputEvent::IE_Released)
		{
			InvertTool = false;
		}

		return true;
	}

	return false;
}

bool FDynamicTerrainMode::UsesToolkits() const
{
	return true;
}

/// Command Functions ///

TSharedRef<FUICommandList> FDynamicTerrainMode::GetCommandList() const
{
	check(Toolkit.IsValid());
	return Toolkit->GetToolkitCommands();
}

TerrainModeID FDynamicTerrainMode::GetMode()
{
	return CurrentMode->ModeID;
}

const FName FDynamicTerrainMode::GetModeName()
{
	return CurrentMode->ModeName;
}

void FDynamicTerrainMode::SetMode(TerrainModeID ModeID)
{
	if (ModeID != TerrainModeID::NUM)
	{
		CurrentMode = Modes[(int)ModeID];
	}
	FString out = "Mode changed to: ";
	out.Append(CurrentMode->ModeName.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, out);
}

FToolSet* FDynamicTerrainMode::GetTools()
{
	return &Tools;
}