#include "DynamicTerrainMode.h"
#include "DynamicTerrainModeToolkit.h"

#include "EngineUtils.h"
#include "EditorViewportClient.h"

#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "TerrainMode"

const FEditorModeID FDynamicTerrainMode::DynamicTerrainModeID = TEXT("DynamicTerrainModeID");

FDynamicTerrainMode::FDynamicTerrainMode()
{
	Settings = NewObject<UDynamicTerrainSettings>(GetTransientPackage(), TEXT("DynamicTerrainSettings"), RF_Transactional);

	// Create editor mode
	Modes.SetNum((int)TerrainModeID::NUM);
	Modes[(int)TerrainModeID::CREATE] = new FDynamicTerrainToolMode("Create", TerrainModeID::CREATE);
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

	if (Terrain == nullptr)
	{
		// Try to select a terrain if one isn't selected
		for (TActorIterator<ATerrain> itr(GetWorld()); itr; ++itr)
		{
			Terrain = *itr;
			break;
		}
	}

	if (Terrain == nullptr)
	{
		// Create a terrain if one still isn't selected
		SetMode(TerrainModeID::CREATE);
	}
	else if (CurrentMode->ModeID == TerrainModeID::SCULPT)
	{
		Terrain->ShowBrush(true);
	}

	ToolUpdate();
	ModeUpdate();

	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, "Terrain mode");
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

	if (CurrentMode->ModeID == TerrainModeID::SCULPT)
	{
		if (Terrain != nullptr)
		{
			FTerrainTool* tool = Tools.GetTool();

			// Adjust the brush display
			Terrain->SetBrushPosition(hit.Location);
			Terrain->SetBrushSize(tool->Size, tool->Falloff);

			if (MouseClick)
			{
				// Apply the tool
				tool->Invert = InvertTool;
				tool->Apply(Terrain, hit.Location, DeltaTime);

				// Force the terrain to update
				Terrain->Update();
			}
		}
	}
	else if (CurrentMode->ModeID == TerrainModeID::MANAGE)
	{
		// Select a terrain when clicked in manage mode
		if (MouseClick)
		{
			ATerrain* newterrain = Cast<ATerrain>(hit.Actor);
			if (newterrain != nullptr && newterrain != Terrain)
			{
				Terrain = newterrain;
				ModeUpdate();
			}
		}
	}
}

void FDynamicTerrainMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	if (CurrentMode == nullptr)
		return;

	FLinearColor color_edge = FLinearColor::Green;
	FLinearColor color_poly = FLinearColor::Green;
	color_poly.G /= 2.0f;

	FVector scale(100.0f);
	FVector center(0.0f);

	if (CurrentMode->ModeID == TerrainModeID::MANAGE || CurrentMode->ModeID == TerrainModeID::CREATE)
	{
		// Draw gridlines on the selected terrain or at the location of a potential new terrain
		if (CurrentMode->ModeID == TerrainModeID::MANAGE)
		{
			if (Terrain != nullptr)
			{
				scale = Terrain->GetActorScale3D();
				center = Terrain->GetActorLocation();
			}
			else
			{
				return;
			}
		}

		int32 polygons = Settings->ComponentSize - 1;
		float offset_x = (float)(Settings->WidthX * polygons) * scale.X / 2.0f;
		float offset_y = (float)(Settings->WidthY * polygons) * scale.Y / 2.0f;

		// Draw X axis lines
		for (int32 x = 0; x <= Settings->WidthX * polygons; ++x)
		{
			FVector start = center;
			start.X -= offset_x - x * scale.X;
			FVector end = start;
			start.Y -= offset_y;
			end.Y += offset_y;

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
			start.Y -= offset_y - y * scale.Y;
			FVector end = start;
			start.X -= offset_x;
			end.X += offset_x;

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
	else
	{
		// Highlight the selected terrain
		if (Terrain != nullptr)
		{
			scale = Terrain->GetActorScale3D();
			center = Terrain->GetActorLocation();

			float offset_x = (float)(Terrain->GetXWidth() * (Terrain->GetComponentSize() - 1)) * scale.X / 2.0f;
			float offset_y = (float)(Terrain->GetYWidth() * (Terrain->GetComponentSize() - 1)) * scale.Y / 2.0f;

			FVector p00(center.X - offset_x, center.Y - offset_y, center.Z);
			FVector p10(center.X + offset_x, center.Y - offset_y, center.Z);
			FVector p01(center.X - offset_x, center.Y + offset_y, center.Z);
			FVector p11(center.X + offset_x, center.Y + offset_y, center.Z);

			PDI->DrawLine(p00, p10, color_edge, ESceneDepthPriorityGroup::SDPG_World, 10.0f);
			PDI->DrawLine(p10, p11, color_edge, ESceneDepthPriorityGroup::SDPG_World, 10.0f);
			PDI->DrawLine(p11, p01, color_edge, ESceneDepthPriorityGroup::SDPG_World, 10.0f);
			PDI->DrawLine(p01, p00, color_edge, ESceneDepthPriorityGroup::SDPG_World, 10.0f);
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
			Viewport->CaptureMouse(true);

			MouseClick = true;
		}
		else if (Event == EInputEvent::IE_Released)
		{
			Viewport->CaptureMouse(false);

			MouseClick = false;
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
			// Enable the brush decal
			if (Terrain != nullptr)
			{
				Terrain->ShowBrush(true);
			}
		}
		else
		{
			if (Terrain != nullptr)
			{
				Terrain->ShowBrush(false);

				if (ModeID == TerrainModeID::MANAGE)
				{
					ModeUpdate();
				}
			}

			if (ModeID == TerrainModeID::CREATE)
			{
				// Set the details panel to defaul values
				Settings->ComponentSize = 64;
				Settings->WidthX = 3;
				Settings->WidthY = 3;
				Settings->UVTiling = 1.0f;
				Settings->Border = true;
			}
		}
	}

	((FDynamicTerrainModeToolkit*)GetToolkit().Get())->RefreshDetails();
}

FToolSet* FDynamicTerrainMode::GetTools()
{
	return &Tools;
}

ATerrain* FDynamicTerrainMode::GetSelected()
{
	return Terrain;
}

void FDynamicTerrainMode::ToolUpdate()
{
	// Update settings panel
	Settings->Strength = Tools.GetTool()->Strength;
	Settings->Size = Tools.GetTool()->Size;
	Settings->Falloff = Tools.GetTool()->Falloff;

	((FDynamicTerrainModeToolkit*)GetToolkit().Get())->RefreshDetails();
}

void FDynamicTerrainMode::ModeUpdate()
{
	if (Terrain != nullptr)
	{
		// Update settings panel
		Settings->ComponentSize = Terrain->GetComponentSize();
		Settings->WidthX = Terrain->GetXWidth();
		Settings->WidthY = Terrain->GetYWidth();
		Settings->UVTiling = Terrain->GetTiling();
		Settings->Border = Terrain->GetBorderEnabled();

		((FDynamicTerrainModeToolkit*)GetToolkit().Get())->RefreshDetails();
	}
}

void FDynamicTerrainMode::ResizeTerrain()
{
	if (Terrain != nullptr)
	{
		Terrain->SetTiling(Settings->UVTiling);
		Terrain->EnableBorder(Settings->Border);
		Terrain->Resize(Settings->ComponentSize, Settings->WidthX, Settings->WidthY);
	}
}

void FDynamicTerrainMode::CreateTerrain()
{
	GEditor->BeginTransaction(LOCTEXT("CreateTerrainTransaction", "New Terrain"));

	// Spawn the terrain
	ATerrain* new_terrain = Cast<ATerrain>(GetWorld()->SpawnActor(ATerrain::StaticClass()));

	// Resize the new terrain
	new_terrain->SetTiling(Settings->UVTiling);
	new_terrain->EnableBorder(Settings->Border);
	new_terrain->Resize(Settings->ComponentSize, Settings->WidthX, Settings->WidthY);

	Terrain = new_terrain;

	GEditor->EndTransaction();
}

#undef LOCTEXT_NAMESPACE