#include "HeightMap.h"
#include "Algorithms.h"

#include "ProceduralMeshComponent.h"

#include <chrono>

/// Blueprint Functions ///

void UHeightMap::Resize(int32 X, int32 Y, int32 Z)
{
	if (X <= 0 || Y <= 0 || Z <= 0)
	{
		return;
	}

	WidthX = X;
	WidthY = Y;
	MaxHeight = Z;

	MapData.SetNumZeroed(WidthX * WidthY, true);
}

float UHeightMap::BPGetHeight(int32 X, int32 Y) const
{
	if (X < 0 || Y < 0)
	{
		return 0.0f;
	}

	return GetHeight(X, Y);
}

void UHeightMap::CalculateNormalsAndTangents(int32 MinX, int32 MinY, int32 MaxX, int32 MaxY, TArray<FVector>& Normals, TArray<FProcMeshTangent>& Tangents) const
{
	// Resize the normal and tangent arrays
	int32 map_size_x = MaxX - MinX;
	int32 map_size_y = MaxY - MinY;

	Normals.SetNum(map_size_x * map_size_y);
	Tangents.SetNum(map_size_x * map_size_y);

	// Calculate normals
	for (int32 y = 0; y < map_size_y; ++y)
	{
		for (int32 x = 0; x < map_size_x; ++x)
		{
			int32 map_offset_x = MinX + x;
			int32 map_offset_y = MinY + y;
			float s01 = GetHeight(map_offset_x - 1, map_offset_y) * MaxHeight;
			float s21 = GetHeight(map_offset_x + 1, map_offset_y) * MaxHeight;
			float s10 = GetHeight(map_offset_x, map_offset_y - 1) * MaxHeight;
			float s12 = GetHeight(map_offset_x, map_offset_y + 1) * MaxHeight;

			// Get tangents in the x and y directions
			FVector vx(2.0f, 0, s21 - s01);
			FVector vy(0, 2.0f, s10 - s12);

			// Calculate the cross product of the two tangents
			vx.Normalize();
			vy.Normalize();
			Normals[y * map_size_x + x] = FVector::CrossProduct(vx, vy);
			Tangents[y * map_size_x + x] = FProcMeshTangent(vx.X, vx.Y, vx.Z);
		}
	}
}

/// Native Functions ///

float UHeightMap::GetHeight(uint32 X, uint32 Y) const
{
	return MapData[Y * WidthX + X];
}

void UHeightMap::SetHeight(uint32 X, uint32 Y, float Height)
{
	MapData[Y * WidthX + X] = Height;
}

int32 UHeightMap::GetWidthX() const
{
	return WidthX;
}

int32 UHeightMap::GetWidthY() const
{
	return WidthY;
}

/// Map Generator Functions ///

void UMapGenerator::Flat(UHeightMap* Map)
{
	for (int32 i = 0; i < Map->GetWidthX(); ++i)
	{
		for (int32 j = 0; j < Map->GetWidthY(); ++j)
		{
			Map->SetHeight(i, j, 0.0f);
		}
	}
}

void UMapGenerator::Plasma(UHeightMap* Map, int32 Scale)
{
	// Safety check for input values
	if (Scale < 1)
	{
		Scale = 1;
	}

	unsigned seed = (unsigned)std::chrono::steady_clock::now().time_since_epoch().count();

	int32 width_x = Map->GetWidthX();
	int32 width_y = Map->GetWidthY();

	// Create the plasma noise
	PlasmaNoise noise(Scale, seed);
	noise.scale(width_x, width_y);

	// Sample the noise onto the terrain
	for (int32 x = 0; x < width_x; ++x)
	{
		for (int32 y = 0; y < width_y; ++y)
		{
			Map->SetHeight(x, y, noise.cubic((float)x, (float)y));
		}
	}
}

void UMapGenerator::Perlin(UHeightMap* Map, int32 Frequency, int32 Octaves, float Persistence)
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

	unsigned seed = (unsigned)std::chrono::steady_clock::now().time_since_epoch().count();

	int32 width_x = Map->GetWidthX();
	int32 width_y = Map->GetWidthY();

	// Create noise data
	std::vector<GradientNoise> noise;
	for (int32 i = 1; i <= Octaves; ++i)
	{
		noise.push_back(GradientNoise(Frequency * i, Frequency * i, seed++));
		noise.back().scale(width_x, width_y);
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
				height += noise[i].perlin(x, y) * amplitude;
				total += amplitude;
				amplitude *= Persistence;
			}

			Map->SetHeight(x, y, height / total);
		}
	}
}