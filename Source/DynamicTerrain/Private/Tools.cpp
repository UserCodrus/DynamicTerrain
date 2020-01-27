#include "Tools.h"

using namespace TerrainTool;

#define LOCTEXT_NAMESPACE "TerrainTools"

/// Raise Tool ///

void Raise::Use(UHeightMap* Map, int32 X, int32 Y)
{
	if (X < 0 || Y < 0 || X >= Map->GetWidthX() || Y >= Map->GetWidthY())
	{
		return;
	}


}

void Raise::Activate()
{

}

void Raise::Deactivate()
{

}

void Raise::Tick()
{
	
}

FText Raise::GetName()
{
	return LOCTEXT("TerrianToolRaiseName", "Raise Terrain");
}

#undef LOCTEXT_NAMESPACE