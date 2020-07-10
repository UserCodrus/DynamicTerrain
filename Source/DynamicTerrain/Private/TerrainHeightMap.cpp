#include "TerrainHeightMap.h"

/// Blueprint Functions ///

void UHeightMap::Resize(int32 X, int32 Y)
{
	if (X <= 0 || Y <= 0)
	{
		return;
	}

	WidthX = X;
	WidthY = Y;

	MapData.Empty();
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

/// Native Functions ///

void UHeightMap::GetMapSection(FMapSection* Section, FIntPoint Min)
{
	// Check to ensure the section is allocated and won't be outside the bounds of the heightmap
	if (Section->X < 2 || Section->Y < 2 || Section->Data.Num() != Section->X * Section->Y)
		return;
	if (Min.X < 0 || Min.Y < 0 || Min.X + Section->X > WidthX || Min.Y + Section->Y > WidthY)
		return;

	// Fill the map data
	int32 i = 0;
	for (int32 y = Min.Y; y < Min.Y + Section->Y; ++y)
	{
		for (int32 x = Min.X; x < Min.X + Section->X; ++x)
		{
			Section->Data[i] = MapData[y * WidthX + x];
			++i;
		}
	}
}

float UHeightMap::GetHeight(uint32 X, uint32 Y) const
{
	return MapData[Y * WidthX + X];
}

float UHeightMap::GetLinearHeight(float X, float Y) const
{
	// Separate the whole number and fractional portions of the coordinates
	uint32 _X = X;
	uint32 _Y = Y;
	X -= _X;
	Y -= _Y;

	// Interpolate the heights at the four corners of the cell containing X, Y
	return FMath::Lerp(
		FMath::Lerp(MapData[_Y * WidthX + _X], MapData[_Y * WidthX + _X + 1], X),
		FMath::Lerp(MapData[(_Y + 1) * WidthX + _X], MapData[(_Y + 1) * WidthX + _X + 1], X),
		Y);
}

FVector UHeightMap::GetNormal(uint32 X, uint32 Y) const
{
	float s01 = MapData[X - 1 + Y * WidthX];
	float s21 = MapData[X + 1 + Y * WidthX];
	float s10 = MapData[X + (Y - 1) * WidthX];
	float s12 = MapData[X + (Y + 1) * WidthX];

	// Get tangents in the x and y directions
	FVector vx(2.0f, 0.0f, s21 - s01);
	FVector vy(0.0f, 2.0f, s12 - s10);

	// Calculate the cross product of the two tangents
	vx.Normalize();
	vy.Normalize();

	return FVector::CrossProduct(vx, vy);
}

FVector UHeightMap::GetLinearNormal(float X, float Y) const
{
	// Separate the whole number and fractional portions of the coordinates
	uint32 _X = X;
	uint32 _Y = Y;
	X -= _X;
	Y -= _Y;

	// Get the height on the edges
	float s01 = FMath::Lerp(MapData[_Y * WidthX + _X], MapData[(_Y + 1) * WidthX + _X], Y);
	float s21 = FMath::Lerp(MapData[_Y * WidthX + _X + 1], MapData[(_Y + 1) * WidthX + _X + 1], Y);
	float s10 = FMath::Lerp(MapData[_Y * WidthX + _X], MapData[_Y * WidthX + _X + 1], X);
	float s12 = FMath::Lerp(MapData[(_Y + 1) * WidthX + _X], MapData[(_Y + 1) * WidthX + _X + 1], X);

	// Get tangents in the X and Y directions
	FVector vx(2.0f, 0, s21 - s01);
	FVector vy(0, 2.0f, s12 - s10);

	// Calculate the cross product of the two tangents
	vx.Normalize();
	vy.Normalize();

	return FVector::CrossProduct(vx, vy);
}

FVector UHeightMap::GetTangent(uint32 X, uint32 Y) const
{
	float s01 = MapData[X - 1 + Y * WidthX];
	float s21 = MapData[X + 1 + Y * WidthX];
	float s10 = MapData[X + (Y - 1) * WidthX];
	float s12 = MapData[X + (Y + 1) * WidthX];

	// Get tangents in the x and y directions
	FVector vx(2.0f, 0, s21 - s01);
	FVector vy(0, 2.0f, s12 - s10);

	// Return the x tangent
	vx.Normalize();
	return vx;
}

FVector UHeightMap::GetLinearTangent(float X, float Y) const
{
	// Separate the whole number and fractional portions of the coordinates
	uint32 _X = X;
	uint32 _Y = Y;
	X -= _X;
	Y -= _Y;

	// Get the height on the edges
	float s01 = FMath::Lerp(MapData[_Y * WidthX + _X], MapData[(_Y + 1) * WidthX + _X], Y);
	float s21 = FMath::Lerp(MapData[_Y * WidthX + _X + 1], MapData[(_Y + 1) * WidthX + _X + 1], Y);
	float s10 = FMath::Lerp(MapData[_Y * WidthX + _X], MapData[_Y * WidthX + _X + 1], X);
	float s12 = FMath::Lerp(MapData[(_Y + 1) * WidthX + _X], MapData[(_Y + 1) * WidthX + _X + 1], X);

	// Get tangent in the X direction
	FVector vx(2.0f, 0, s21 - s01);
	vx.Normalize();
	return vx;
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