#include "DynamicTerrainMode.h"
#include "DynamicTerrainModeToolkit.h"

#include "EngineUtils.h"
#include "EditorViewportClient.h"

#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "TerrainMode"

const FEditorModeID FDynamicTerrainMode::DynamicTerrainModeID = TEXT("DynamicTerrainModeID");

ABrushProxy::ABrushProxy()
{
	// Create the decal
	Decal = CreateDefaultSubobject<UBrushDecal>(TEXT("TerrainEditorBrushDecal"));
	Decal->SetVisibility(false);

	SetActorLabel("BrushProxy");
}

bool ABrushProxy::IsSelectable() const
{
	return false;
}

void ABrushProxy::ShowBrush(bool Visible)
{
	Decal->SetVisibility(Visible);
}

void ABrushProxy::SetBrush(FVector Location, FTerrainTool* Tool, ATerrain* Terrain)
{
	Decal->SetRelativeLocation(Location);
	Decal->Resize(Tool, Terrain);
}

FDynamicTerrainMode::FDynamicTerrainMode()
{
	Settings = NewObject<UDynamicTerrainSettings>(GetTransientPackage(), TEXT("DynamicTerrainSettings"));
	Settings->AddToRoot();

	MapGen = NewObject<UMapGenerator>(GetTransientPackage(), TEXT("DynamicTerrainMapGenerator"));
	MapGen->AddToRoot();

	// Create editor modes
	Modes.SetNum((int)TerrainModeID::NUM);
	Modes[(int)TerrainModeID::CREATE] = new FDynamicTerrainToolMode("Create", TerrainModeID::CREATE);
	Modes[(int)TerrainModeID::SCULPT] = new FDynamicTerrainToolMode("Sculpt", TerrainModeID::SCULPT);
	Modes[(int)TerrainModeID::MANAGE] = new FDynamicTerrainToolMode("Manage", TerrainModeID::MANAGE);
	Modes[(int)TerrainModeID::GENERATE] = new FDynamicTerrainToolMode("Generate", TerrainModeID::GENERATE);

	CurrentMode = Modes[1];

	// Create terrain generator data
	for (int32 i = 0; i < 10; ++i)
	{
		Settings->IntProperties[i] = 0;
		Settings->FloatProperties[i] = 0.0f;
	}

	// Get the names of all the generator functions
	TArray<FName> fnames;
	UMapGenerator::StaticClass()->GenerateFunctionList(fnames);

	for (int32 i = 0; i < fnames.Num(); ++i)
	{
		Generators.Add(MakeShareable(new FTerrainGenerator(fnames[i])));

		// Get the function parameters for the generator function
		UFunction* funct = UMapGenerator::StaticClass()->FindFunctionByName(fnames[i]);
		if (funct != nullptr)
		{
			for (TFieldIterator<UProperty> it = TFieldIterator<UProperty>(funct); it; ++it)
			{
				UProperty* prop = *it;

				// Make sure the property is a numeric type
				if (UNumericProperty* nprop = Cast<UNumericProperty>(prop))
				{
					// Get the name and type of the parameter
					Generators.Last()->IsFloat.Push(nprop->IsFloatingPoint());
					Generators.Last()->Parameters.Push(nprop->GetName());
				}
				else
				{
					// Ignore the generator if it has non-numeric properties
					Generators.Pop();
					break;
				}
			}
		}
	}

	CurrentGenerator = Generators.Last();
}

FDynamicTerrainMode::~FDynamicTerrainMode()
{
	for (int32 i = 0; i < Modes.Num(); ++i)
	{
		delete Modes[i];
	}
	Modes.Empty();

	Settings->RemoveFromRoot();
	MapGen->RemoveFromRoot();
	MapGen->Map = nullptr;
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

	// Create the brush display
	FActorSpawnParameters spawn_params;
	spawn_params.ObjectFlags = EObjectFlags::RF_Transient;
	Brush = GetWorld()->SpawnActor<ABrushProxy>(spawn_params);

	// Select a terrain from the level, preferably one matching the name of the last terrain selected in this mode
	for (TActorIterator<ATerrain> itr(GetWorld()); itr; ++itr)
	{
		SelectedTerrain = *itr;
		MapGen->Map = SelectedTerrain->GetMap();
		if (SelectedTerrain->GetName() == TerrainName)
		{
			break;
		}
	}

	if (SelectedTerrain == nullptr)
	{
		// If no terrain is found, switch to create mode
		SetMode(TerrainModeID::CREATE);
		TerrainName = "";
	}
	else
	{
		if (CurrentMode->ModeID == TerrainModeID::SCULPT)
		{
			Brush->ShowBrush(true);
		}

		TerrainName = SelectedTerrain->GetName();
	}

	ModeUpdate();
	ToolUpdate();
}

void FDynamicTerrainMode::Exit()
{
	// Deselect the terrain to prevent dangling pointerse
	SelectedTerrain = nullptr;
	MapGen->Map = nullptr;

	// Destroy the brush proxy
	Brush->Destroy();
	Brush = nullptr;

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
		ATerrain* terrain = Cast<ATerrain>(hit.Actor);
		if (terrain != nullptr && terrain != SelectedTerrain)
		{
			// Select a terrain when the mouse hovers over it
			SelectTerrain(terrain);
		}

		if (SelectedTerrain != nullptr)
		{
			FTerrainTool* tool = Tools.GetTool();

			// Adjust the brush display
			Brush->SetBrush(hit.Location, tool, SelectedTerrain);

			if (MouseClick)
			{
				// Apply the tool
				tool->Invert = InvertTool;
				tool->Apply(SelectedTerrain, hit.Location, DeltaTime);

				// Force the terrain to update
				SelectedTerrain->Update();
			}
		}
	}
	else if (CurrentMode->ModeID == TerrainModeID::MANAGE || CurrentMode->ModeID == TerrainModeID::GENERATE)
	{
		// Select a terrain when clicked in manage mode
		if (MouseClick)
		{
			ATerrain* newterrain = Cast<ATerrain>(hit.Actor);
			if (newterrain != nullptr && newterrain != SelectedTerrain)
			{
				SelectTerrain(newterrain);
			}
		}
	}
}

void FDynamicTerrainMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	if (CurrentMode == nullptr || Settings == nullptr)
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
			if (SelectedTerrain != nullptr)
			{
				scale = SelectedTerrain->GetActorScale3D();
				center = SelectedTerrain->GetActorLocation();
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
		if (SelectedTerrain != nullptr)
		{
			scale = SelectedTerrain->GetActorScale3D();
			center = SelectedTerrain->GetActorLocation();

			float offset_x = (float)(SelectedTerrain->GetXWidth() * (SelectedTerrain->GetComponentSize() - 1)) * scale.X / 2.0f;
			float offset_y = (float)(SelectedTerrain->GetYWidth() * (SelectedTerrain->GetComponentSize() - 1)) * scale.Y / 2.0f;

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
			if (SelectedTerrain != nullptr)
			{
				Brush->ShowBrush(true);
			}
		}
		else
		{
			if (SelectedTerrain != nullptr)
			{
				Brush->ShowBrush(false);

				if (ModeID == TerrainModeID::MANAGE)
				{
					ModeUpdate();
				}
			}

			if (ModeID == TerrainModeID::CREATE)
			{
				ModeUpdate();
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
	return SelectedTerrain;
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
	if (SelectedTerrain != nullptr && CurrentMode->ModeID != TerrainModeID::CREATE)
	{
		// Copy the current terrain's attributes to the settings panel
		Settings->ComponentSize = SelectedTerrain->GetComponentSize();
		Settings->WidthX = SelectedTerrain->GetXWidth();
		Settings->WidthY = SelectedTerrain->GetYWidth();
		Settings->UVTiling = SelectedTerrain->GetTiling();
	}
	else
	{
		// Change the settings to default
		Settings->ComponentSize = 64;
		Settings->WidthX = 3;
		Settings->WidthY = 3;
		Settings->UVTiling = 1.0f;
	}

	((FDynamicTerrainModeToolkit*)GetToolkit().Get())->RefreshDetails();
}

void FDynamicTerrainMode::ResizeTerrain()
{
	if (SelectedTerrain != nullptr)
	{
		SelectedTerrain->SetTiling(Settings->UVTiling);
		SelectedTerrain->Resize(Settings->ComponentSize, Settings->WidthX, Settings->WidthY);
	}
}

void FDynamicTerrainMode::CreateTerrain()
{
	GEditor->BeginTransaction(LOCTEXT("CreateTerrainTransaction", "New Terrain"));

	// Spawn the terrain
	ATerrain* new_terrain = Cast<ATerrain>(GetWorld()->SpawnActor(ATerrain::StaticClass()));

	// Resize the new terrain
	new_terrain->SetTiling(Settings->UVTiling);
	new_terrain->Resize(Settings->ComponentSize, Settings->WidthX, Settings->WidthY);

	SelectTerrain(new_terrain);

	GEditor->EndTransaction();
}

void FDynamicTerrainMode::SelectTerrain(ATerrain* Terrain)
{
	SelectedTerrain = Terrain;
	TerrainName = SelectedTerrain->GetName();
	MapGen->Map = SelectedTerrain->GetMap();

	ModeUpdate();
}

void FDynamicTerrainMode::ProcessGenerateCommand(/*const TCHAR* Command*/)
{
	if (MapGen->Map == nullptr || CurrentGenerator == nullptr)
		return;

	// Create a console command using parameters from the settings panel
	FString command(CurrentGenerator->Name.ToString());

	for (int32 i = 0; i < CurrentGenerator->IsFloat.Num(); ++i)
	{
		command.Append(" ");

		if (CurrentGenerator->IsFloat[i])
		{
			command.Append(FString::SanitizeFloat(Settings->FloatProperties[i]));
		}
		else
		{
			command.AppendInt(Settings->IntProperties[i]);
		}
	}

	// Run the script command passed into the function
	MapGen->CallFunctionByNameWithArguments(*command, *GetGlobalLogSingleton(), MapGen, true);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, command);
	SelectedTerrain->Refresh();
}

void FDynamicTerrainMode::SelectGenerator(TSharedPtr<FTerrainGenerator> Generator)
{
	if (Generator != nullptr)
	{
		CurrentGenerator = Generator;
		((FDynamicTerrainModeToolkit*)GetToolkit().Get())->RefreshDetails();
	}
}

TSharedPtr<FTerrainGenerator> FDynamicTerrainMode::GetGenerator()
{
	return CurrentGenerator;
}

#undef LOCTEXT_NAMESPACE