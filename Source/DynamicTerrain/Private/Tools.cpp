#include "Tools.h"

#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "TerrainTools"

float FTerrainTool::TraceDistance = 10000;

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

bool FTerrainTool::GetCursorOnTerrain(FHitResult& Result)
{
	if (Terrain != nullptr)
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

			if (UGameplayStatics::DeprojectScreenToWorld(world->GetFirstPlayerController(), Mouse, WorldOrigin, WorldDirection))
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
					Map->SetHeight(x, y, height + GetMagnitude(dist) * Delta);
				}
			}
		}
	}
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