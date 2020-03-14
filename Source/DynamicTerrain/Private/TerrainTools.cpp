#include "TerrainTools.h"

#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "TerrainTools"

const FName FSculptTool::ToolID = TEXT("SculptTool");
const FName FSmoothTool::ToolID = TEXT("SmoothTool");
const FName FFlattenTool::ToolID = TEXT("FlattenTool");

/// Terrain Brushes ///

FText FBrushLinear::GetName() const
{
	return LOCTEXT("TerrianBrushLinearName", "Linear Brush");
}

TerrainBrushID FBrushLinear::GetID() const
{
	return TerrainBrushID::LINEAR;
}

float FBrushLinear::GetStrength(float Distance, float Radius, float Falloff) const
{
	return Distance < Radius ? 1.0f : Falloff > 0.0f ? FMath::Max(0.0f, 1.0f - (Distance - Radius) / Falloff) : 0.0f;
}

FText FBrushSmooth::GetName() const
{
	return LOCTEXT("TerrianBrushSmoothName", "Smooth Brush");
}

TerrainBrushID FBrushSmooth::GetID() const
{
	return TerrainBrushID::SMOOTH;
}

float FBrushSmooth::GetStrength(float Distance, float Radius, float Falloff) const
{
	float s = Distance < Radius ? 1.0f : Falloff > 0.0f ? FMath::Max(0.0f, 1.0f - (Distance - Radius) / Falloff) : 0.0f;
	return s * s * (3 - 2 * s);
}

FText FBrushRound::GetName() const
{
	return LOCTEXT("TerrianBrushRoundName", "Round Brush");
}

TerrainBrushID FBrushRound::GetID() const
{
	return TerrainBrushID::ROUND;
}

float FBrushRound::GetStrength(float Distance, float Radius, float Falloff) const
{
	float s = Distance < Radius ? 0.0f : Falloff > 0.0f ? FMath::Min(1.0f, (Distance - Radius) / Falloff) : 1.0f;
	return 1.0f - s * s;
}

FText FBrushSphere::GetName() const
{
	return LOCTEXT("TerrianBrushSphereName", "Sphere Brush");
}

TerrainBrushID FBrushSphere::GetID() const
{
	return TerrainBrushID::SPHERE;
}

float FBrushSphere::GetStrength(float Distance, float Radius, float Falloff) const
{
	float s = Distance < Radius ? 0.0f : Falloff > 0.0f ? FMath::Min(1.0f, (Distance - Radius) / Falloff) : 1.0f;
	return FMath::Sqrt(1.0f - s * s);
}

/// Terrain Tools ///

float FTerrainTool::TraceDistance = 50000;

void FTerrainTool::SetBrush(FTerrainBrush* NewBrush)
{
	Brush = NewBrush;
}

void FTerrainTool::Apply(ATerrain* Terrain, FVector Center, float Delta) const
{
	FVector2D MapCenter = WorldVectorToMapVector(Terrain, Center);

	int32 width_x = Terrain->GetMap()->GetWidthX();
	int32 width_y = Terrain->GetMap()->GetWidthY();
	if (MapCenter.X < 0 || MapCenter.Y < 0 || MapCenter.X > width_x || MapCenter.Y > width_y)
	{
		return;
	}

	// Get the mask for the tool
	FBrushStroke mask = GetStroke(Terrain->GetMap(), MapCenter);
	FIntRect bounds = mask.GetBounds();
	
	// Apply the mask to the heightmap
	for (int x = bounds.Min.X; x < bounds.Max.X; ++x)
	{
		for (int y = bounds.Min.Y; y < bounds.Max.Y; ++y)
		{
			// Change the vertex height
			float height = Terrain->GetMap()->GetHeight(x, y);
			Terrain->GetMap()->SetHeight(x, y, height + mask.GetData(x, y) * Delta * Strength);
		}
	}

	// Update the terrain sections covered
	Terrain->UpdateRange(bounds);
}

void FTerrainTool::Apply(UHeightMap* Map, FVector2D Center, float Delta) const
{
	int32 width_x = Map->GetWidthX();
	int32 width_y = Map->GetWidthY();
	if (Center.X < 0 || Center.Y < 0 || Center.X > width_x || Center.Y > width_y)
	{
		return;
	}

	// Get the mask for the tool
	FBrushStroke mask = GetStroke(Map, Center);
	FIntRect bounds = mask.GetBounds();

	// Apply the mask to the heightmap
	for (int x = bounds.Min.X; x < bounds.Max.X; ++x)
	{
		for (int y = bounds.Min.Y; y < bounds.Max.Y; ++y)
		{
			// Change the vertex height
			float height = Map->GetHeight(x, y);
			Map->SetHeight(x, y, height + mask.GetData(x, y) * Delta * Strength);
		}
	}
}

bool FTerrainTool::MouseToTerrainPosition(ATerrain* Terrain, const FSceneView* View, FHitResult& Result) const
{
	if (Terrain != nullptr && View != nullptr)
	{
		UWorld* world = Terrain->GetWorld();

		// Get the location of the mouse in the active game viewport
		UGameViewportClient* viewport = world->GetGameViewport();

		FVector2D Mouse;
		if (viewport->GetMousePosition(Mouse))
		{
			// Project the mouse into the world
			FVector WorldOrigin;
			FVector WorldDirection;

			FSceneView::DeprojectScreenToWorld(Mouse, View->UnconstrainedViewRect, View->ViewMatrices.GetInvViewProjectionMatrix(), WorldOrigin, WorldDirection);

			// Trace from the viewport outward under the cursor
			FHitResult hit;
			world->LineTraceSingleByChannel(hit, WorldOrigin, WorldOrigin + WorldDirection * TraceDistance, ECollisionChannel::ECC_WorldDynamic);

			AActor* actor = hit.GetActor();
			if (actor != nullptr)
			{
				if (actor->IsA<ATerrain>())
				{
					Result = hit;
					return true;
				}
			}
		}
	}

	// Return no result if anything failed
	Result = FHitResult();
	return false;
}

bool FTerrainTool::MouseToTerrainPosition(ATerrain* Terrain, const APlayerController* Controller, FHitResult& Result) const
{
	if (Terrain != nullptr && Controller != nullptr)
	{
		UWorld* world = Terrain->GetWorld();

		// Get the location of the mouse in the active game viewport
		UGameViewportClient* viewport = world->GetGameViewport();

		FVector2D Mouse;
		if (viewport->GetMousePosition(Mouse))
		{
			// Project the mouse into the world
			FVector WorldOrigin;
			FVector WorldDirection;

			if (UGameplayStatics::DeprojectScreenToWorld(Controller, Mouse, WorldOrigin, WorldDirection))
			{
				// Trace from the viewport outward under the cursor
				FHitResult hit;
				world->LineTraceSingleByChannel(hit, WorldOrigin, WorldOrigin + WorldDirection * TraceDistance, ECollisionChannel::ECC_WorldDynamic);

				Result = hit;
				return true;
			}
		}
	}

	// Return no result if anything failed
	Result = FHitResult();
	return false;
}

FVector2D FTerrainTool::WorldVectorToMapVector(ATerrain* Terrain, FVector WorldPosition) const
{
	if (Terrain == nullptr)
		return FVector2D();

	// Get the corner of the terrain corresponding to 0, 0 on the heightmap
	float xwidth = (float)(Terrain->GetMap()->GetWidthX() - 3) / 2.0f;
	float ywidth = (float)(Terrain->GetMap()->GetWidthY() - 3) / 2.0f;
	FVector scale = Terrain->GetActorScale();

	FVector corner = Terrain->GetActorLocation();
	corner.X -= xwidth * scale.X;
	corner.Y += ywidth * scale.Y;

	// Get the heightmap coordinates of the point
	FVector2D loc;
	loc.X = (WorldPosition.X - corner.X) / scale.X + 1.0f;
	loc.Y = -(WorldPosition.Y - corner.Y) / scale.Y + 1.0f;

	return loc;
}

/// Sculpt Tool ///

FBrushStroke FSculptTool::GetStroke(UHeightMap* Map, FVector2D Center) const
{
	// Get the outer bounds of the circle
	FIntRect bounds;
	bounds.Min.X = FMath::FloorToInt(Center.X - Size - Falloff);
	bounds.Max.X = FMath::CeilToInt(Center.X + Size + Falloff);
	bounds.Min.Y = FMath::FloorToInt(Center.Y - Size - Falloff);
	bounds.Max.Y = FMath::CeilToInt(Center.Y + Size + Falloff);

	// Keep the circle within the bounds of the heightmap
	if (bounds.Min.X < 0)
	{
		bounds.Min.X = 0;
	}
	if (bounds.Min.Y < 0)
	{
		bounds.Min.Y = 0;
	}
	if (bounds.Max.X > Map->GetWidthX())
	{
		bounds.Max.X = Map->GetWidthX();
	}
	if (bounds.Max.Y > Map->GetWidthY())
	{
		bounds.Max.Y = Map->GetWidthY();
	}

	// Invert the mask
	float inversion = 1.0f;
	if (Invert)
	{
		inversion = -1.0f;
	}

	// Calculate the brusk mask using the current brush
	FBrushStroke stroke(bounds);
	for (int x = bounds.Min.X; x < bounds.Max.X; ++x)
	{
		for (int y = bounds.Min.Y; y < bounds.Max.Y; ++y)
		{
			// Set the mask based on the current brush
			float dist = FVector2D::Distance(Center, FVector2D(x, y));
			stroke.GetData(x, y) = Brush->GetStrength(dist, Size, Falloff) * inversion;
		}
	}

	return stroke;
}

FText FSculptTool::GetName() const
{
	return LOCTEXT("TerrianToolSculptName", "Sculpt Terrain");
}

FName FSculptTool::GetToolID() const
{
	return ToolID;
}

TerrainToolID FSculptTool::GetID() const
{
	return TerrainToolID::SCULPT;
}

/// Smooth Tool ///

FBrushStroke FSmoothTool::GetStroke(UHeightMap* Map, FVector2D Center) const
{
	// Get the outer bounds of the circle
	FIntRect bounds;
	bounds.Min.X = FMath::FloorToInt(Center.X - Size - Falloff);
	bounds.Max.X = FMath::CeilToInt(Center.X + Size + Falloff);
	bounds.Min.Y = FMath::FloorToInt(Center.Y - Size - Falloff);
	bounds.Max.Y = FMath::CeilToInt(Center.Y + Size + Falloff);

	// Keep the circle within the bounds of the heightmap
	if (bounds.Min.X < 1)
	{
		bounds.Min.X = 1;
	}
	if (bounds.Min.Y < 1)
	{
		bounds.Min.Y = 1;
	}
	if (bounds.Max.X > Map->GetWidthX() - 1)
	{
		bounds.Max.X = Map->GetWidthX() - 1;
	}
	if (bounds.Max.Y > Map->GetWidthY() - 1)
	{
		bounds.Max.Y = Map->GetWidthY() - 1;
	}

	// Invert the mask
	float inversion = 1.0f;
	if (Invert)
	{
		inversion = -1.0f;
	}

	// Calculate the brusk mask using the current brush
	FBrushStroke stroke(bounds);
	for (int x = bounds.Min.X; x < bounds.Max.X; ++x)
	{
		for (int y = bounds.Min.Y; y < bounds.Max.Y; ++y)
		{
			float dist = FVector2D::Distance(Center, FVector2D(x, y));

			// Set the mask to the difference between the average height of the surrounding area and the current point
			float h00 = Map->GetHeight(x, y);
			float h01 = Map->GetHeight(x - 1, y);
			float h21 = Map->GetHeight(x + 1, y);
			float h10 = Map->GetHeight(x, y - 1);
			float h12 = Map->GetHeight(x, y + 1);

			float hdiff = (h01 + h21 + h10 + h12) / 4.0f - h00;

			stroke.GetData(x, y) = 20.0f * hdiff * Brush->GetStrength(dist, Size, Falloff) * inversion;
		}
	}

	return stroke;
}

FText FSmoothTool::GetName() const
{
	return LOCTEXT("TerrianToolSmoothName", "Smooth Terrain");
}

FName FSmoothTool::GetToolID() const
{
	return ToolID;
}

TerrainToolID FSmoothTool::GetID() const
{
	return TerrainToolID::SMOOTH;
}

/// Flatten Tool ///

FBrushStroke FFlattenTool::GetStroke(UHeightMap* Map, FVector2D Center) const
{
	// Get the outer bounds of the circle
	FIntRect bounds;
	bounds.Min.X = FMath::FloorToInt(Center.X - Size - Falloff);
	bounds.Max.X = FMath::CeilToInt(Center.X + Size + Falloff);
	bounds.Min.Y = FMath::FloorToInt(Center.Y - Size - Falloff);
	bounds.Max.Y = FMath::CeilToInt(Center.Y + Size + Falloff);

	// Keep the circle within the bounds of the heightmap
	if (bounds.Min.X < 0)
	{
		bounds.Min.X = 0;
	}
	if (bounds.Min.Y < 0)
	{
		bounds.Min.Y = 0;
	}
	if (bounds.Max.X > Map->GetWidthX())
	{
		bounds.Max.X = Map->GetWidthX();
	}
	if (bounds.Max.Y > Map->GetWidthY())
	{
		bounds.Max.Y = Map->GetWidthY();
	}

	// Invert the mask
	float inversion = 1.0f;
	if (Invert)
	{
		inversion = -1.0f;
	}

	// Get the height at the center point
	int X = (int)Center.X;
	int Y = (int)Center.Y;
	float DX = Center.X - X;
	float DY = Center.Y - Y;
	float height = FMath::Lerp(
		FMath::Lerp(Map->GetHeight(X, Y), Map->GetHeight(X + 1, Y), DX), 
		FMath::Lerp(Map->GetHeight(X, Y + 1), Map->GetHeight(X + 1, Y + 1), DX),
		DY);

	// Calculate the brusk mask using the current brush
	FBrushStroke stroke(bounds);
	for (int x = bounds.Min.X; x < bounds.Max.X; ++x)
	{
		for (int y = bounds.Min.Y; y < bounds.Max.Y; ++y)
		{
			float dist = FVector2D::Distance(Center, FVector2D(x, y));

			// Set the mask to the difference between the center height and the current point
			stroke.GetData(x, y) = (height - Map->GetHeight(x, y)) * Brush->GetStrength(dist, Size, Falloff) * inversion;
		}
	}

	return stroke;
}

FText FFlattenTool::GetName() const
{
	return LOCTEXT("TerrianToolFlattenName", "Flatten Terrain");
}

FName FFlattenTool::GetToolID() const
{
	return ToolID;
}

TerrainToolID FFlattenTool::GetID() const
{
	return TerrainToolID::FLATTEN;
}

/// Tool and Brush Sets ///

FToolSet::FToolSet()
{
	// Create tools
	Tools.SetNum((int)TerrainToolID::NUM);
	Tools[(int)TerrainToolID::SCULPT] = new FSculptTool;
	Tools[(int)TerrainToolID::SMOOTH] = new FSmoothTool;
	Tools[(int)TerrainToolID::FLATTEN] = new FFlattenTool;

	ActiveTool = Tools[0];

	// Create brushes
	Brushes.SetNum((int)TerrainBrushID::NUM);
	Brushes[(int)TerrainBrushID::LINEAR] = new FBrushLinear;
	Brushes[(int)TerrainBrushID::SMOOTH] = new FBrushSmooth;
	Brushes[(int)TerrainBrushID::ROUND] = new FBrushRound;
	Brushes[(int)TerrainBrushID::SPHERE] = new FBrushSphere;

	ActiveBrush = Brushes[0];
	ActiveTool->SetBrush(ActiveBrush);
}

FToolSet::~FToolSet()
{
	for (int32 i = 0; i < Tools.Num(); ++i)
	{
		delete Tools[i];
	}
	Tools.Empty();

	for (int32 i = 0; i < Brushes.Num(); ++i)
	{
		delete Brushes[i];
	}
	Brushes.Empty();
}

void FToolSet::SetTool(TerrainToolID Tool)
{
	if (Tool != TerrainToolID::NUM)
	{
		ActiveTool = Tools[(int)Tool];
		ActiveTool->SetBrush(ActiveBrush);
	}
}

FTerrainTool* FToolSet::GetTool()
{
	return ActiveTool;
}

TerrainToolID FToolSet::ToolID()
{
	return ActiveTool->GetID();
}

// Set the active brush
void FToolSet::SetBrush(TerrainBrushID Brush)
{
	if (Brush != TerrainBrushID::NUM)
	{
		ActiveBrush = Brushes[(int)Brush];
		ActiveTool->SetBrush(ActiveBrush);
	}
}
// Get the active brush
FTerrainBrush* FToolSet::GetBrush()
{
	return ActiveBrush;
}
// Get the active brush's ID
TerrainBrushID FToolSet::BrushID()
{
	return ActiveBrush->GetID();
}

#undef LOCTEXT_NAMESPACE