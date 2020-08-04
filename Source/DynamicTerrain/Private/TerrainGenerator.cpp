#include "TerrainGenerator.h"
#include "Terrain.h"
#include "TerrainAlgorithms.h"

#include "Kismet/KismetMathLibrary.h"

#include <chrono>
#include <random>
#include <limits>

/// Map Generator Functions ///

void UMapGenerator::NewSeed()
{
	Seed = (uint32)std::chrono::steady_clock::now().time_since_epoch().count();
}

void UMapGenerator::SetSeed(int32 NewSeed)
{
	Seed = (uint32)NewSeed;
}

void UMapGenerator::Flat(float Height)
{
	MapFlat(Height);
}

void UMapGenerator::Plasma(int32 Scale, int32 Foliage, float MaxHeight)
{
	MapPlasma(Scale, MaxHeight);
	FoliageUniform(Foliage, Foliage);
}

void UMapGenerator::Perlin(int32 Frequency, int32 Octaves, float Persistence, float MaxHeight)
{
	MapPerlin(Frequency, Octaves, Persistence, MaxHeight);
	FoliageRandom(Frequency * 10);
}

/// Map Generator Components ///

void UMapGenerator::MapFlat(float Height)
{
	UHeightMap* Map = Terrain->GetMap();
	for (int32 i = 0; i < Map->GetWidthX(); ++i)
	{
		for (int32 j = 0; j < Map->GetWidthY(); ++j)
		{
			Map->SetHeight(i, j, Height);
		}
	}
}

void UMapGenerator::MapPlasma(int32 Scale, float MaxHeight)
{
	// Safety check for input values
	if (Scale < 1)
	{
		Scale = 1;
	}

	UHeightMap* Map = Terrain->GetMap();

	int32 width_x = Map->GetWidthX();
	int32 width_y = Map->GetWidthY();

	// Create the plasma noise
	PlasmaNoise noise(Scale, Seed);
	noise.Scale(width_x, width_y);

	// Sample the noise onto the terrain
	for (int32 x = 0; x < width_x; ++x)
	{
		for (int32 y = 0; y < width_y; ++y)
		{
			Map->SetHeight(x, y, noise.Cubic((float)x, (float)y) * MaxHeight);
		}
	}
}

void UMapGenerator::MapPerlin(int32 Frequency, int32 Octaves, float Persistence, float MaxHeight)
{
	// Safety check for input values
	if (Frequency < 2)
	{
		Frequency = 2;
	}
	if (Octaves < 1)
	{
		Octaves = 1;
	}
	if (Persistence < 0.0f)
	{
		Persistence = 0.0f;
	}
	else if (Persistence > 1.0f)
	{
		Persistence = 1.0f;
	}

	UHeightMap* Map = Terrain->GetMap();

	int32 width_x = Map->GetWidthX();
	int32 width_y = Map->GetWidthY();

	// Create noise data
	std::vector<GradientNoise> noise;
	for (int32 i = 1; i <= Octaves; ++i)
	{
		noise.push_back(GradientNoise(Frequency * i, Frequency * i, Seed++));
		noise.back().Scale(width_x, width_y);
	}

	// Sample the noise onto the terrain
	for (int32 x = 0; x < width_x; ++x)
	{
		for (int32 y = 0; y < width_y; ++y)
		{
			float amplitude = 1.0f;
			float total = 0.0f;
			float height = 0.0f;
			for (int32 i = 0; i < Octaves; ++i)
			{
				height += noise[i].Perlin(x, y) * amplitude;
				total += amplitude;
				amplitude *= Persistence;
			}

			Map->SetHeight(x, y, height * MaxHeight / total);
		}
	}
}

void UMapGenerator::FoliageRandom(uint32 NumPoints)
{
	if (NumPoints < 1)
		return;

	TArray<UTerrainFoliageGroup*> groups;
	Terrain->GetFoliageGroups(groups);
	std::default_random_engine rando(Seed);
	std::uniform_int_distribution<uint32> random_seed(0, std::numeric_limits<uint32>::max());

	for (int32 i = 0; i < groups.Num(); ++i)
	{
		// Create noise
		PointNoise noise(2, 2, NumPoints, random_seed(rando));
		const TArray<FVector2D>& points = noise.GetPoints();

		// Add foliage objects
		float xoffset = (float)(Terrain->GetMap()->GetWidthX() - 3) / 2.0f * Terrain->GetActorScale3D().X;
		float yoffset = (float)(Terrain->GetMap()->GetWidthY() - 3) / 2.0f * Terrain->GetActorScale3D().Y;
		for (int32 p = 0; p < points.Num(); ++p)
		{
			// Get the location of the foliage in world space
			FVector location = Terrain->GetActorLocation();
			location.X += (points[p].X - 1.0f) * xoffset;
			location.Y += (points[p].Y - 1.0f) * yoffset;

			// Place the foliage object
			groups[i]->AddFoliageCluster(Terrain, location, random_seed(rando));
		}
	}
}

void UMapGenerator::FoliageUniform(uint32 XPoints, uint32 YPoints)
{
	if (XPoints < 1 || YPoints < 1)
		return;

	TArray<UTerrainFoliageGroup*> groups;
	Terrain->GetFoliageGroups(groups);
	std::default_random_engine rando(Seed);
	std::uniform_int_distribution<uint32> random_seed(0, std::numeric_limits<uint32>::max());

	for (int32 i = 0; i < groups.Num(); ++i)
	{
		// Create noise
		UniformPointNoise noise(XPoints, YPoints, random_seed(rando));
		const TArray<FVector2D>& points = noise.GetPoints();

		// Add foliage objects
		float xoffset = (float)(Terrain->GetMap()->GetWidthX() - 3) / 2.0f * Terrain->GetActorScale3D().X;
		float yoffset = (float)(Terrain->GetMap()->GetWidthY() - 3) / 2.0f * Terrain->GetActorScale3D().Y;
		for (int32 p = 0; p < points.Num(); ++p)
		{
			// Get the location of the foliage in world space
			FVector location = Terrain->GetActorLocation();
			location.X += ((points[p].X / noise.GetWidth()) * 2.0f - 1.0f) * xoffset;
			location.Y += ((points[p].Y / noise.GetHeight()) * 2.0f - 1.0f) * yoffset;

			// Place the foliage object
			groups[i]->AddFoliageCluster(Terrain, location, random_seed(rando));
		}
	}
}