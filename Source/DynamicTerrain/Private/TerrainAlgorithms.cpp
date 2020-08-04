#include "TerrainAlgorithms.h"

#include <random>

constexpr float pi = 3.141592f;

/// Utility Functions ///

// Linear interpolation
inline float lerp(float t, float a, float b)
{
	return a + t * (b - a);
}

// Cosine interpolation
inline float corp(float t, float a, float b)
{
	float u = (1 - std::cos(t * pi)) / 2;
	return a * (1 - u) + b * u;
}

// Cubic interpolation
inline float curp(float t, float a[4])
{
	return a[1] + 0.5f * t * (a[2] - a[0] + t * (2.0f * a[0] - 5.0f * a[1] + 4.0f * a[2] - a[3] + t * (3.0f * (a[1] - a[2]) + a[3] - a[0])));
}

// Perlin smoothstep
inline float fade(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

/// Base Noise ///

uint32 Noise::GetWidth() const
{
	return Width;
}

uint32 Noise::GetHeight() const
{
	return Height;
}

/// Gradient Noise ///

GradientNoise::GradientNoise(uint32 NewWidth, uint32 NewHeight, uint32 Seed)
{
	Width = NewWidth;
	Height = NewHeight;
	Gradient = new FVector2D[Width * Height];

	// Generate gradient vectors
	std::default_random_engine rando(Seed);
	std::uniform_real_distribution<float> dist(-pi, pi);
	for (unsigned y = 0; y < Height; ++y)
	{
		for (unsigned x = 0; x < Width; ++x)
		{
			// Create a unit vector from a random angle
			float angle = dist(rando);
			Gradient[y * Width + x].X = cos(angle);
			Gradient[y * Width + x].Y = sin(angle);
		}
	}
}

GradientNoise::GradientNoise(const GradientNoise& Copy)
{
	Width = Copy.Width;
	Height = Copy.Height;
	Gradient = new FVector2D[Width * Height];
	memcpy(Gradient, Copy.Gradient, Width * Height * sizeof(FVector2D));
}

GradientNoise::~GradientNoise()
{
	if (Gradient != nullptr)
	{
		delete[] Gradient;
	}
}

void GradientNoise::Scale(uint32 SampleWidth, uint32 SampleHeight)
{
	ScaleX = (float)(Width - 1) / SampleWidth;
	ScaleY = (float)(Height - 1) / SampleHeight;
}

FVector2D GradientNoise::GetGradient(uint32 X, uint32 Y) const
{
	return Gradient[Y * Width + X];
}

float GradientNoise::Perlin(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the coordinates of the grid cell containing x, y
	int32 x = (int32)X;
	int32 y = (int32)Y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	X -= x;
	Y -= y;

	// Get the fade curves of the coordinates
	float u = fade(X);
	float v = fade(Y);

	// Get the gradients at the corner of the unit cell
	FVector2D g00 = Gradient[y * Width + x];
	FVector2D g01 = Gradient[(y + 1) * Width + x];
	FVector2D g10 = Gradient[y * Width + x + 1];
	FVector2D g11 = Gradient[(y + 1) * Width + x + 1];

	// Interpolate the dot products of each gradient and the cell coordinates
	return lerp(u,
		lerp(v, g00.X * X + g00.Y * Y,			g01.X * X + g01.Y * (Y - 1)),
		lerp(v, g10.X * (X - 1) + g10.Y * Y,	g11.X * (X - 1) + g11.Y * (Y - 1))
	);
}

/// Value Noise ///

ValueNoise::ValueNoise(uint32 NewWidth, uint32 NewHeight, uint32 Seed)
{
	Width = NewWidth;
	Height = NewHeight;
	Value = new float[Width * Height];

	// Generate random values at each grid point
	std::default_random_engine rando(Seed);
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	for (unsigned y = 0; y < Height; ++y)
	{
		for (unsigned x = 0; x < Width; ++x)
		{
			Value[y * Width + x] = dist(rando);
		}
	}
}

ValueNoise::ValueNoise(const ValueNoise& Copy)
{
	Width = Copy.Width;
	Height = Copy.Height;
	Value = new float[Width * Height];
	memcpy(Value, Copy.Value, Width * Height * sizeof(float));
}

ValueNoise::~ValueNoise()
{
	if (Value != nullptr)
	{
		delete[] Value;
	}
}

void ValueNoise::Scale(uint32 SampleWidth, uint32 SampleHeight)
{
	ScaleX = (float)(Width - 1) / SampleWidth;
	ScaleY = (float)(Height - 1) / SampleHeight;
}

float ValueNoise::GetValue(uint32 X, uint32 Y) const
{
	return Value[Y * Width + X];
}

float ValueNoise::Linear(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the coordinates of the grid cell containing x, y
	int32 x = (int32)X;
	int32 y = (int32)Y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	X -= x;
	Y -= y;

	// Interpolate the noise
	return lerp(X,
		lerp(Y, Value[y * Width + x],			Value[(y + 1) * Width + x]),
		lerp(Y, Value[y * Width + (x + 1)],		Value[(y + 1) * Width + (x + 1)])
	);
}

float ValueNoise::Cosine(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the coordinates of the grid cell containing x, y
	int32 x = (int32)X;
	int32 y = (int32)Y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	X -= x;
	Y -= y;

	// Interpolate the noise
	return corp(X,
		corp(Y, Value[y * Width + x], Value[(y + 1) * Width + x]),
		corp(Y, Value[y * Width + (x + 1)], Value[(y + 1) * Width + (x + 1)])
	);
}

float ValueNoise::Cubic(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the coordinates of the grid cell containing x, y
	int32 x = (int32)X;
	int32 y = (int32)Y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	X -= x;
	Y -= y;

	float a[4], p[4];

	// Second point
	p[1] = Value[y * Width + x];
	p[2] = Value[y * Width + (x + 1)];
	if (x > 0)
	{
		p[0] = Value[y * Width + (x - 1)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[0] = p[1] - (p[2] - p[1]);
	}
	if (x < (int)(Width - 2))
	{
		p[3] = Value[y * Width + (x + 2)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[3] = p[2] + (p[2] - p[1]);
	}
	a[1] = curp(X, p);

	// Third point
	p[1] = Value[(y + 1) * Width + x];
	p[2] = Value[(y + 1) * Width + (x + 1)];
	if (x > 0)
	{
		p[0] = Value[(y + 1) * Width + (x - 1)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[0] = p[1] - (p[2] - p[1]);
	}
	if (x < (int)(Width - 2))
	{
		p[3] = Value[(y + 1) * Width + (x + 2)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[3] = p[2] + (p[2] - p[1]);
	}
	a[2] = curp(X, p);

	// First point
	if (y > 0)
	{
		p[1] = Value[(y - 1) * Width + x];
		p[2] = Value[(y - 1) * Width + (x + 1)];
		if (x > 0)
		{
			p[0] = Value[(y - 1) * Width + (x - 1)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[0] = p[1] - (p[2] - p[1]);
		}
		if (x < (int)(Width - 2))
		{
			p[3] = Value[(y - 1) * Width + (x + 2)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[3] = p[2] + (p[2] - p[1]);
		}
		a[0] = curp(X, p);
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		a[0] = a[1] - (a[2] - a[1]);
	}

	// Fourth point
	if (y < (int)(Height - 2))
	{
		p[1] = Value[(y + 2) * Width + x];
		p[2] = Value[(y + 2) * Width + (x + 1)];
		if (x > 0)
		{
			p[0] = Value[(y + 2) * Width + (x - 1)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[0] = p[1] - (p[2] - p[1]);
		}
		if (x < (int)(Width - 2))
		{
			p[3] = Value[(y + 2) * Width + (x + 2)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[3] = p[2] + (p[2] - p[1]);
		}
		a[3] = curp(X, p);
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		a[3] = a[2] + (a[2] - a[1]);
	}

	return std::min(1.0f, std::max(curp(Y, a), -1.0f));
}

/// Plasma Noise ///

PlasmaNoise::PlasmaNoise(uint32 Size, uint32 Seed)
{
	Width = (unsigned)pow(2, Size) + 1;
	Height = Width;
	Value = new float[Width * Height];

	std::default_random_engine rando(Seed);
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	// The range of the random values generated
	float range = 0.5;

	// Generate corner values
	Value[0] = dist(rando) * range;
	Value[Width - 1] = dist(rando) * range;
	Value[(Height - 1) * Width] = dist(rando) * range;
	Value[Height * Width - 1] = dist(rando) * range;

	// The size of the current fractal
	unsigned stride = Width - 1;

	unsigned limit_x = Width - 1;
	unsigned limit_y = Height - 1;

	// Diamond-square algorithm
	while (stride > 1)
	{
		range *= 0.5;
		unsigned half = stride / 2;

		// Diamond step
		for (unsigned y = 0; y < limit_y; y += stride)
		{
			for (unsigned x = 0; x < limit_x; x += stride)
			{
				// Get the values of the four grid points at the corner of the current grid point
				float v00 = Value[y * Width + x];
				float v10 = Value[y * Width + (x + stride)];
				float v01 = Value[(y + stride) * Width + x];
				float v11 = Value[(y + stride) * Width + (x + stride)];

				// Set the value of the current point to the average of the corners + a random value
				Value[(y + half) * Width + (x + half)] = (v00 + v10 + v01 + v11) / 4.0f + dist(rando) * range;
			}
		}

		// Square step - top / bottom edges
		for (unsigned x = half; x < limit_x; x += stride)
		{
			// Get the values for the points adjacent to the top edge
			float v_mid = Value[half * Width + x];
			float v_left = Value[x - half];
			float v_right = Value[x + half];

			// Set the value for the top edge at the current x coordinate to the average of the adjacent points + a random value
			Value[x] = (v_mid + v_left + v_right) / 3.0f + dist(rando) * range;

			// Get the values for the points adjacent to the bottom edge
			v_mid = Value[(limit_y - half) * Width + x];
			v_left = Value[limit_y * Width + (x - half)];
			v_right = Value[limit_y * Width + (x + half)];

			// Set the value for the bottom edge at the current x coordinate to the average of the adjacent points + a random value
			Value[limit_y * Width + x] = (v_mid + v_left + v_right) / 3.0f + dist(rando) * range;
		}

		// Square step - left / right edges
		for (unsigned y = half; y < limit_y; y += stride)
		{
			// Get the values for the points adjacent to the left edge
			float v_mid = Value[y * Width + half];
			float v_top = Value[(y - half) * Width];
			float v_bottom = Value[(y + half) * Width];

			// Set the value for the left edge at the current y coordinate to the average of the adjacent points + a random value
			Value[y * Width] = (v_mid + v_top + v_bottom) / 3.0f + dist(rando) * range;

			// Get the values for the points adjacent to the right edge
			v_mid = Value[y * Width + (limit_x - half)];
			v_top = Value[(y - half) * Width + limit_x];
			v_bottom = Value[(y + half) * Width + limit_x];

			// Set the value for the right edge at the current y coordinate to the average of the adjacent points + a random value
			Value[y * Width + limit_x] = (v_mid + v_top + v_bottom) / 3.0f + dist(rando) * range;
		}

		// Square step - center points
		bool offset = true;
		for (unsigned y = half; y < limit_y; y += half)
		{
			for (unsigned x = offset ? stride : half; x < limit_x; x += stride)
			{
				// Get the values of the four grid points adjacent the current grid point
				float v_top = Value[(y - half) * Width + x];
				float v_bottom = Value[(y + half) * Width + x];
				float v_left = Value[y * Width + (x - half)];
				float v_right = Value[y * Width + (x + half)];

				// Set the value of the current point to the average of the adjacent points + a random value
				Value[y * Width + x] = (v_top + v_bottom + v_left + v_right) / 4.0f + dist(rando) * range;
			}

			offset = !offset;
		}

		stride /= 2;
	}
}

PlasmaNoise::PlasmaNoise(const PlasmaNoise& Copy)
{
	Width = Copy.Width;
	Height = Copy.Height;
	Value = new float[Width * Height];
	memcpy(Value, Copy.Value, Width * Height * sizeof(float));
}

/// Random Point Noise ///

PointNoise::PointNoise(uint32 XWidth, uint32 YWidth, uint32 NumPoints, uint32 Seed)
{
	Width = XWidth;
	Height = YWidth;

	// Generate points using random x and y values
	std::default_random_engine rando(Seed);
	std::uniform_real_distribution<float> x_dist(0.0f, (float)Width);
	std::uniform_real_distribution<float> y_dist(0.0f, (float)Height);

	Points.SetNumUninitialized(NumPoints);
	for (uint32 i = 0; i < NumPoints; ++i)
	{
		// Create a point and sort it in the point grid
		Points[i] = FVector2D(x_dist(rando), y_dist(rando));
	}
}

PointNoise::PointNoise(uint32 Radius, uint32 NumPoints, uint32 Seed)
{
	Width = Radius * 2;
	Height = Width;

	// Generate random x and y values within a circle
	std::default_random_engine rando(Seed);
	std::uniform_real_distribution<float> angle(0.0f, 360.0f);
	std::uniform_real_distribution<float> dist;

	Points.SetNumUninitialized(NumPoints);
	for (uint32 i = 0; i < NumPoints; ++i)
	{
		// Get a random angle and distance
		float a = angle(rando);
		float d = Radius * FMath::Sqrt(dist(rando));

		// Create a point in the circle and sort it into the point grid
		Points[i] = FVector2D(d * FMath::Cos(a), d * FMath::Sin(a));
	}
}

void PointNoise::Scale(uint32 SampleWidth, uint32 SampleHeight)
{
	ScaleX = (float)Width / SampleWidth;
	ScaleY = (float)Height / SampleHeight;
}

FVector2D PointNoise::GetNearest(FVector2D Location) const
{
	// The closest point found so far
	FVector2D nearest(0.0f, 0.0f);
	// The distance to the closest point
	float nearest_distance = 8.0f;

	// Check each point to see which one is closest
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		float distance = FVector2D::DistSquared(Location, Points[i]);
		if (distance < nearest_distance)
		{
			nearest_distance = distance;
			nearest = Points[i];
		}
	}

	return nearest;
}

float PointNoise::GetNearestDistance(FVector2D Location) const
{
	// The distance to the closest point
	float nearest_distance = 8.0f;

	// Check each point to get the distance to the nearest one
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		float distance = FVector2D::DistSquared(Location, Points[i]);
		if (distance < nearest_distance)
		{
			nearest_distance = distance;
		}
	}

	return FMath::Sqrt(nearest_distance);
}

float PointNoise::Dot(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the nearest neighbor to the current point
	FVector2D loc(X, Y);
	float nearest = GetNearestDistance(loc);

	// Calculate the noise value based on distance
	return std::max(0.0f, 1.0f - nearest * 4);
}

float PointNoise::Worley(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the nearest neighbor to the current point
	FVector2D loc(X, Y);
	float nearest = GetNearestDistance(loc);

	// Calculate the noise value based on distance
	return std::min(1.0f, nearest);
}

const TArray<FVector2D>& PointNoise::GetPoints()
{
	return Points;
}

/// Grid Aligned Point Noise ///

UniformPointNoise::UniformPointNoise(uint32 NewWidth, uint32 NewHeight, uint32 Seed)
{
	Width = NewWidth;
	Height = NewHeight;

	// Generate points at random locations within a unit grid
	std::default_random_engine rando(Seed);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	Points.SetNumUninitialized(Width * Height);
	for (uint32 y = 0; y < Height; ++y)
	{
		for (uint32 x = 0; x < Width; ++x)
		{
			uint32 loc = x + y * Width;
			Points[loc].X = x + dist(rando);
			Points[loc].Y = y + dist(rando);
		}
	}
}

inline FVector2D UniformPointNoise::GetNearest(FVector2D Location) const
{
	// Get the grid location to the top left of the current point
	int32 X = (int32)Location.X - 1;
	int32 Y = (int32)Location.Y - 1;

	// The closest point found so far
	FVector2D nearest(0.0f, 0.0f);
	// The distance to the closest point
	float nearest_distance = 8.0f;

	// Check each grid cell surrounding the cell containing to current point
	for (uint32 y = 0; y < 3; ++y)
	{
		int32 offset_y = (Y + y) * Width;
		for (uint32 x = 0; x < 3; ++x)
		{
			// Get the cell's location in the point array
			int32 cell = (X + x) + offset_y;
			if (cell > -1 && cell < Points.Num())
			{
				// Check the distance to the point
				float dist = FVector2D::DistSquared(Location, Points[cell]);
				if (dist < nearest_distance)
				{
					nearest_distance = dist;
					nearest = Points[cell];
				}
			}
		}
	}

	return nearest;
}

float UniformPointNoise::GetNearestDistance(FVector2D Location) const
{
	// Get the grid location to the top left of the current point
	int32 X = (int32)Location.X - 1;
	int32 Y = (int32)Location.Y - 1;

	// The distance to the closest point
	float nearest_distance = 8.0f;

	// Check each grid cell surrounding the cell containing to current point
	for (uint32 y = 0; y < 3; ++y)
	{
		int32 offset_y = (Y + y) * Width;
		for (uint32 x = 0; x < 3; ++x)
		{
			// Get the cell's location in the point array
			int32 cell = (X + x) + offset_y;
			if (cell > -1 && cell < Points.Num())
			{
				// Check the distance to the point
				float distance = FVector2D::DistSquared(Location, Points[cell]);
				if (distance < nearest_distance)
				{
					nearest_distance = distance;
				}
			}
		}
	}

	return FMath::Sqrt(nearest_distance);
}

float UniformPointNoise::Dot(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the nearest neighbor to the current point
	FVector2D loc(X, Y);
	float nearest = GetNearestDistance(loc);

	return std::max(0.0f, 1.0f - nearest * 4);
}

float UniformPointNoise::Worley(float X, float Y) const
{
	// Scale noise values
	X *= ScaleX;
	Y *= ScaleY;

	// Get the nearest neighbor to the current point
	FVector2D loc(X, Y);
	float nearest = GetNearestDistance(loc);

	return std::min(1.0f, nearest);
}

inline FVector2D UniformPointNoise::GetPoint(uint32 X, uint32 Y) const
{
	return Points[X + Y * Width];
}