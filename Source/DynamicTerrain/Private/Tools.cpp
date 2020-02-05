#include "Tools.h"

#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "TerrainTools"

float FTerrainTool::TraceDistance = 50000;

void FTerrainTool::Activate()
{
	Active = true;
}

void FTerrainTool::Deactivate()
{
	Active = false;
}

void FTerrainTool::Select(ATerrain* Target)
{
	Terrain = Target;
}

bool FTerrainTool::MouseToTerrainPosition(const FSceneView* View, FHitResult& Result)
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

bool FTerrainTool::MouseToTerrainPosition(const APlayerController* Controller, FHitResult& Result)
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
	}

	// Return no result if anything failed
	Result = FHitResult();
	return false;
}

FVector2D FTerrainTool::WorldVectorToMapVector(FVector WorldPosition)
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

float FTerrainTool::GetMagnitude(float Distance)
{
	// Scale the magnitude based on the distance from the center point
	float magnitude = Distance / Size;
	// Scale exponentially
	magnitude = (1.0f - magnitude * magnitude);
	// Apply falloff
	//magnitude = magnitude / Falloff;
	// Apply hardness
	magnitude = magnitude * (1.0f - Hardness) + Hardness;
	// Clamp the value and apply the strength multiplier
	magnitude = FMath::Clamp(magnitude, 0.0f, 1.0f) * Strength;

	return magnitude;
}

/// Raise Tool ///

void FRaiseTool::Use(UHeightMap* Map, FVector2D Center, float Delta)
{
	int32 width_x = Map->GetWidthX();
	int32 width_y = Map->GetWidthY();
	if (Center.X < 0 || Center.Y < 0 || Center.X > width_x || Center.Y > width_y)
	{
		return;
	}

	// Get the outer bounds of the circle
	int minx = (int)(Center.X - Size - 1);
	int maxx = (int)(Center.X + Size + 1);
	int miny = (int)(Center.Y - Size - 1);
	int maxy = (int)(Center.Y + Size + 1);

	if (minx < 0)
	{
		minx = 0;
	}
	if (miny < 0)
	{
		miny = 0;
	}
	if (maxx > width_x)
	{
		maxx = width_x;
	}
	if (maxy > width_y)
	{
		maxy = width_y;
	}

	// Raise each vertex within the circle's radius
	for (int x = minx; x < maxx; ++x)
	{
		for (int y = miny; y < maxy; ++y)
		{
			// Make sure the vertex is within the border of the circle
			float dist = FVector2D::Distance(Center, FVector2D(x, y));

			if (dist <= Size)
			{
				// Raise the vertex height
				float height = Map->GetHeight(x, y);
				Map->SetHeight(x, y, height + GetMagnitude(dist) * Delta);
			}
		}
	}

	// Update the terrain sections covered
	Terrain->UpdateRange(minx, miny, maxx, maxy);
}

void FRaiseTool::Tick(float DeltaTime)
{
	
}

FText FRaiseTool::GetName()
{
	return LOCTEXT("TerrianToolRaiseName", "Raise Terrain");
}

/// Lower Tool ///

void FLowerTool::Use(UHeightMap* Map, FVector2D Center, float Delta)
{
	int32 width_x = Map->GetWidthX();
	int32 width_y = Map->GetWidthY();
	if (Center.X < 0 || Center.Y < 0 || Center.X >= width_x || Center.Y >= width_y)
	{
		return;
	}

	// Set the radius of the circle proportional to the size
	int radius = (int)Size + 1;

	// Raise each vertex within the circle's radius
	for (int32 x = Center.X - radius; x < Center.X + radius; ++x)
	{
		for (int32 y = Center.Y - radius; y < Center.Y + radius; ++y)
		{
			if (x >= 0 || y >= 0 || x < width_x || y < width_y)
			{
				// Make sure the vertex is within the border of the circle
				float dist = FVector2D::Distance(Center, FVector2D(x, y));

				if (dist <= Size)
				{
					// Raise the vertex height
					float height = Map->GetHeight(x, y);
					Map->SetHeight(x, y, height - GetMagnitude(dist) * Delta);
				}
			}
		}
	}
}

void FLowerTool::Tick(float DeltaTime)
{

}

FText FLowerTool::GetName()
{
	return LOCTEXT("TerrianToolRaiseName", "Lower Terrain");
}

#undef LOCTEXT_NAMESPACE