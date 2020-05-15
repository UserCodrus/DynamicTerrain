#include "TerrainToolComponent.h"

#include "Kismet/GameplayStatics.h"

UTerrainToolComponent::UTerrainToolComponent()
{
	BrushDecal = CreateDefaultSubobject<UBrushDecal>(TEXT("TerrainGameplayBrushDecal"));
	BrushDecal->SetupAttachment(this);
	BrushDecal->SetVisibility(false);

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

/// USceneComponent Interface ///

void UTerrainToolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!ToolEnabled)
	{
		// Hide the brush when the tool is disabled
		BrushDecal->SetVisibility(false);
		return;
	}

	// Get the controller of the pawn that owns this component
	APawn* owner = Cast<APawn>(GetOwner());
	if (owner != nullptr)
	{
		APlayerController* controller = Cast<APlayerController>(owner->Controller);
		if (controller != nullptr)
		{
			// Project the mouse into the world
			FVector2D Mouse;
			UWorld* world = GetWorld();
			UGameViewportClient* viewport = world->GetGameViewport();
			if (viewport->GetMousePosition(Mouse))
			{
				FVector WorldOrigin;
				FVector WorldDirection;
				if (UGameplayStatics::DeprojectScreenToWorld(controller, Mouse, WorldOrigin, WorldDirection))
				{
					// Trace from the viewport outward under the cursor
					FHitResult hit;
					world->LineTraceSingleByChannel(hit, WorldOrigin, WorldOrigin + WorldDirection * MaxBrushDistance, ECollisionChannel::ECC_WorldDynamic);

					if (hit.IsValidBlockingHit())
					{
						// Move the brush
						BrushDecal->SetVisibility(true);
						BrushDecal->SetRelativeLocation(hit.Location);

						ATerrain* terrain = Cast<ATerrain>(hit.GetActor());
						if (terrain != nullptr)
						{
							BrushDecal->Resize(TerrainTools.GetTool(), terrain);
							if (ToolActive)
							{
								// Apply the terrain tool
								TerrainTools.GetTool()->Invert = ToolInvert;
								TerrainTools.GetTool()->Apply(terrain, hit.Location, DeltaTime);
							}
						}
					}
					else
					{
						BrushDecal->SetVisibility(false);
					}
				}
			}
		}
	}

}

/// Terrain Tool Component Interface ///

void UTerrainToolComponent::SelectTool(TerrainToolID ToolID)
{
	TerrainTools.SetTool(ToolID);
}

void UTerrainToolComponent::SelectBrush(TerrainBrushID BrushID)
{
	TerrainTools.SetBrush(BrushID);
}

void UTerrainToolComponent::NextTool()
{
	// Iterate the tool ID of the the current tool
	int tool = (int)TerrainTools.ToolID();
	tool++;

	// Set the tool
	if (tool == (int)TerrainToolID::NUM)
	{
		TerrainTools.SetTool(TerrainToolID::SCULPT);
	}
	else
	{
		TerrainTools.SetTool((TerrainToolID)tool);
	}
}

void UTerrainToolComponent::PreviousTool()
{
	// Decrement the tool ID of the current tool
	int tool = (int)TerrainTools.ToolID();
	tool--;

	// Set the tool
	if (tool == -1)
	{
		tool = (int)TerrainToolID::NUM;
		TerrainTools.SetTool((TerrainToolID)(tool - 1));
	}
	else
	{
		TerrainTools.SetTool((TerrainToolID)tool);
	}
}

void UTerrainToolComponent::NextBrush()
{
	int brush = (int)TerrainTools.BrushID();
	brush++;

	if (brush == (int)TerrainBrushID::NUM)
	{
		TerrainTools.SetBrush(TerrainBrushID::LINEAR);
	}
	else
	{
		TerrainTools.SetBrush((TerrainBrushID)brush);
	}
}

void UTerrainToolComponent::PreviousBrush()
{
	int brush = (int)TerrainTools.BrushID();
	brush++;

	if (brush == -1)
	{
		brush = (int)TerrainBrushID::NUM;
		TerrainTools.SetBrush((TerrainBrushID)(brush - 1));
	}
	else
	{
		TerrainTools.SetBrush((TerrainBrushID)brush);
	}
}

FText UTerrainToolComponent::GetToolName()
{
	return TerrainTools.GetTool()->GetName();
}

FText UTerrainToolComponent::GetBrushName()
{
	return TerrainTools.GetBrush()->GetName();
}