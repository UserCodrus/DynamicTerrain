#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FDynamicTerrainStyle
{
public:
	// Create the style singleton
	static void Initialize();
	// Delete the style singleton
	static void Shutdown();

	// Get the style set
	static TSharedPtr<ISlateStyle> Get();
	// Get the name of the style set
	static const FName GetName();

private:
	// The style set singleton instance
	static TSharedPtr<FSlateStyleSet> StyleSet;
};