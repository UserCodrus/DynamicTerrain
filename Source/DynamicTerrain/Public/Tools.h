#pragma once

#include "CoreMinimal.h"
#include "Terrain.h"

namespace TerrainTool
{
	class Tool
	{
	public:
		virtual void Activate() = 0;
		virtual void Deactivate() = 0;

		virtual void Tick() = 0;

		virtual FText GetName() = 0;

	protected:
		ATerrain* target = nullptr;
	};

	class Raise : public Tool
	{
	public:
		static void Use(UHeightMap* Map, int32 X, int32 Y);

		virtual void Activate() override;
		virtual void Deactivate() override;

		virtual void Tick() override;

		virtual FText GetName() override;
	};

}