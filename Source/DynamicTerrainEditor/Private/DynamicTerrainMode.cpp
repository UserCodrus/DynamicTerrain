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
		if (CurrentMode->ModeID == TerrainModeID::SCULPT)
		{
			Terrain->ShowBrush(true);
		}
		ToolUpdate();
		ModeUpdate();
		break;
	}

	SelectNone();
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
		if (CurrentMode->ModeID == TerrainModeID::SCULPT)
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
}

void FDynamicTerrainMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	if (CurrentMode->ModeID == TerrainModeID::MANAGE)
	{
		FLinearColor color_edge = FLinearColor::Green;
		FLinearColor color_poly = FLinearColor::Green;
		color_poly.G /= 2.0f;

		// Get the scale and location of the terrain
		FVector scale(100.0f);
		FVector center(0.0f);
		if (Terrain != nullptr)
		{
			scale = Terrain->GetActorScale3D();
			center = Terrain->GetActorLocation();
		}

		int32 polygons = Settings->ComponentSize - 1;
		float offset_x = (float)(Settings->WidthX * polygons) / 2.0f;
		float offset_y = (float)(Settings->WidthX * polygons) / 2.0f;

		// Draw X axis lines
		for (int32 x = 0; x <= Settings->WidthX * polygons; ++x)
		{
			FVector start = center;
			start.X = (start.X - offset_x + x) * scale.X;
			FVector end = start;
			start.Y -= offset_y * scale.Y;
			end.Y += offset_y * scale.Y;

			// Draw thick lines on component edges and thin lines on polygons
			if (x % polygons == 0)
			{
				PDI->DrawLine(start, end, color_edge, ESceneDepthPriorityGroup::SDPG_MAX, 10.0f);
			}
			else
			{
				PDI->DrawLine(start, end, color_poly, ESceneDepthPriorityGroup::SDPG_MAX, 3.0f);
			}
		}
		// Draw Y axis lines
		for (int32 y = 0; y <= Settings->WidthY * polygons; ++y)
		{
			FVector start = center;
			start.Y = (start.Y - offset_y + y) * scale.Y;
			FVector end = start;
			start.X -= offset_x * scale.X;
			end.X += offset_x * scale.X;

			// Draw thick lines on component edges and thin lines on polygons
			if (y % polygons == 0)
			{
				PDI->DrawLine(start, end, color_edge, ESceneDepthPriorityGroup::SDPG_MAX, 10.0f);
			}
			else
			{
				PDI->DrawLine(start, end, color_poly, ESceneDepthPriorityGroup::SDPG_MAX, 3.0f);
			}
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

void FDynamicTerrainMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdMode::AddReferencedObjects(Collector);

	// Add the editor settings
	Collector.AddReferencedObject(Settings);
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
		if (ModeID == TerrainModeID::SCULPT)
		{
			Terrain->ShowBrush(true);
		}
		else
		{
			Terrain->ShowBrush(false);
		}
	}
	FString out = "Mode changed to: ";
	out.Append(CurrentMode->ModeName.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, out);
}

FToolSet* FDynamicTerrainMode::GetTools()
{
	return &Tools;
}

void FDynamicTerrainMode::ToolUpdate()
{
	// Update settings panel
	Settings->Strength = Tools.GetTool()->Strength;
	Settings->Size = Tools.GetTool()->Size;
	Settings->Falloff = Tools.GetTool()->Falloff;
}

void FDynamicTerrainMode::ModeUpdate()
{
	// Update settings panel
	Settings->ComponentSize = Terrain->GetComponentSize();
	Settings->WidthX = Terrain->GetXWidth();
	Settings->WidthY = Terrain->GetYWidth();
	Settings->UVTiling = Terrain->GetTiling();
	Settings->Border = Terrain->GetBorderEnabled();
}

void FDynamicTerrainMode::UpdateTerrain()
{
	// Resize the terrain
	Terrain->SetTiling(Settings->UVTiling);
	Terrain->EnableBorder(Settings->Border);
	Terrain->Resize(Settings->ComponentSize, Settings->WidthX, Settings->WidthY);
}