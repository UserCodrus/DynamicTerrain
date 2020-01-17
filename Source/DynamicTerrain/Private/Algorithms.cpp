#include "Algorithms.h"

#include <random>
#include <stdexcept>

constexpr float pi = 3.141592f;

///
/// Utility functions
///

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

///
/// Base noise
///

unsigned Noise::getWidth() const
{
	return width;
}

unsigned Noise::getHeight() const
{
	return height;
}

///
/// Perlin and simplex noise
///

GradientNoise::GradientNoise(unsigned _width, unsigned _height, unsigned seed)
{
	width = _width;
	height = _height;
	gradient = new FVector2D[width * height];

	// Generate gradient vectors
	std::default_random_engine rando(seed);
	std::uniform_real_distribution<float> dist(-pi, pi);
	for (unsigned y = 0; y < height; ++y)
	{
		for (unsigned x = 0; x < width; ++x)
		{
			// Create a unit vector from a random angle
			float angle = dist(rando);
			gradient[y * width + x].X = cos(angle);
			gradient[y * width + x].Y = sin(angle);
		}
	}
}

GradientNoise::GradientNoise(const GradientNoise& copy)
{
	width = copy.width;
	height = copy.height;
	gradient = new FVector2D[width * height];
	memcpy(gradient, copy.gradient, width * height * sizeof(FVector2D));
}

GradientNoise::~GradientNoise()
{
	if (gradient != nullptr)
	{
		delete[] gradient;
	}
}

void GradientNoise::scale(unsigned sample_width, unsigned sample_height)
{
	scale_x = (float)(width - 1) / sample_width;
	scale_y = (float)(height - 1) / sample_height;
}

FVector2D GradientNoise::getGradient(unsigned x, unsigned y) const
{
	return gradient[y * width + x];
}

float GradientNoise::perlin(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the coordinates of the grid cell containing x, y
	int X = (int)x;
	int Y = (int)y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	x -= X;
	y -= Y;

	// Get the fade curves of the coordinates
	float u = fade(x);
	float v = fade(y);

	// Get the gradients at the corner of the unit cell
	FVector2D g00 = gradient[Y * width + X];
	FVector2D g01 = gradient[(Y + 1) * width + X];
	FVector2D g10 = gradient[Y * width + X + 1];
	FVector2D g11 = gradient[(Y + 1) * width + X + 1];

	// Interpolate the dot products of each gradient and the cell coordinates
	return lerp(u,
		lerp(v, g00.X * x + g00.Y * y,			g01.X * x + g01.Y * (y - 1)),
		lerp(v, g10.X * (x - 1) + g10.Y * y,	g11.X * (x - 1) + g11.Y * (y - 1))
	);
}

///
/// Value & diamond square noise
///

ValueNoise::ValueNoise(unsigned _width, unsigned _height, unsigned seed)
{
	width = _width;
	height = _height;
	value = new float[width * height];

	// Generate random values at each grid point
	std::default_random_engine rando(seed);
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	for (unsigned y = 0; y < height; ++y)
	{
		for (unsigned x = 0; x < width; ++x)
		{
			value[y * width + x] = dist(rando);
		}
	}
}

ValueNoise::ValueNoise(const ValueNoise& copy)
{
	width = copy.width;
	height = copy.height;
	value = new float[width * height];
	memcpy(value, copy.value, width * height * sizeof(float));
}

ValueNoise::~ValueNoise()
{
	if (value != nullptr)
	{
		delete[] value;
	}
}

void ValueNoise::scale(unsigned sample_width, unsigned sample_height)
{
	scale_x = (float)(width - 1) / sample_width;
	scale_y = (float)(height - 1) / sample_height;
}

float ValueNoise::getValue(unsigned x, unsigned y) const
{
	return value[y * width + x];
}

float ValueNoise::linear(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the coordinates of the grid cell containing x, y
	int X = (int)x;
	int Y = (int)y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	x -= X;
	y -= Y;

	// Interpolate the noise
	return lerp(x,
		lerp(y, value[Y * width + X],			value[(Y + 1) * width + X]),
		lerp(y, value[Y * width + (X + 1)],		value[(Y + 1) * width + (X + 1)])
	);
}

float ValueNoise::cosine(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the coordinates of the grid cell containing x, y
	int X = (int)x;
	int Y = (int)y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	x -= X;
	y -= Y;

	// Interpolate the noise
	return corp(x,
		corp(y, value[Y * width + X], value[(Y + 1) * width + X]),
		corp(y, value[Y * width + (X + 1)], value[(Y + 1) * width + (X + 1)])
	);
}

float ValueNoise::cubic(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the coordinates of the grid cell containing x, y
	int X = (int)x;
	int Y = (int)y;

	// Subtract the cell coordinates from x and y to get their fractional portion
	x -= X;
	y -= Y;

	float a[4], p[4];

	// Second point
	p[1] = value[Y * width + X];
	p[2] = value[Y * width + (X + 1)];
	if (X > 0)
	{
		p[0] = value[Y * width + (X - 1)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[0] = p[1] - (p[2] - p[1]);
	}
	if (X < (int)(width - 2))
	{
		p[3] = value[Y * width + (X + 2)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[3] = p[2] + (p[2] - p[1]);
	}
	a[1] = curp(x, p);

	// Third point
	p[1] = value[(Y + 1) * width + X];
	p[2] = value[(Y + 1) * width + (X + 1)];
	if (X > 0)
	{
		p[0] = value[(Y + 1) * width + (X - 1)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[0] = p[1] - (p[2] - p[1]);
	}
	if (X < (int)(width - 2))
	{
		p[3] = value[(Y + 1) * width + (X + 2)];
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		p[3] = p[2] + (p[2] - p[1]);
	}
	a[2] = curp(x, p);

	// First point
	if (Y > 0)
	{
		p[1] = value[(Y - 1) * width + X];
		p[2] = value[(Y - 1) * width + (X + 1)];
		if (X > 0)
		{
			p[0] = value[(Y - 1) * width + (X - 1)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[0] = p[1] - (p[2] - p[1]);
		}
		if (X < (int)(width - 2))
		{
			p[3] = value[(Y - 1) * width + (X + 2)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[3] = p[2] + (p[2] - p[1]);
		}
		a[0] = curp(x, p);
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		a[0] = a[1] - (a[2] - a[1]);
	}

	// Fourth point
	if (Y < (int)(height - 2))
	{
		p[1] = value[(Y + 2) * width + X];
		p[2] = value[(Y + 2) * width + (X + 1)];
		if (X > 0)
		{
			p[0] = value[(Y + 2) * width + (X - 1)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[0] = p[1] - (p[2] - p[1]);
		}
		if (X < (int)(width - 2))
		{
			p[3] = value[(Y + 2) * width + (X + 2)];
		}
		else
		{
			// Extrapolate the middle points if we are near an edge
			p[3] = p[2] + (p[2] - p[1]);
		}
		a[3] = curp(x, p);
	}
	else
	{
		// Extrapolate the middle points if we are near an edge
		a[3] = a[2] + (a[2] - a[1]);
	}

	return std::min(1.0f, std::max(curp(y, a), -1.0f));
}

PlasmaNoise::PlasmaNoise(unsigned size, unsigned seed)
{
	width = (unsigned)pow(2, size) + 1;
	height = width;
	value = new float[width * height];

	std::default_random_engine rando(seed);
	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	// The range of the random values generated
	float range = 0.5;

	// Generate corner values
	value[0] = dist(rando) * range;
	value[width - 1] = dist(rando) * range;
	value[(height - 1) * width] = dist(rando) * range;
	value[height * width - 1] = dist(rando) * range;

	// The size of the current fractal
	unsigned stride = width - 1;

	unsigned limit_x = width - 1;
	unsigned limit_y = height - 1;

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
				float v00 = value[y * width + x];
				float v10 = value[y * width + (x + stride)];
				float v01 = value[(y + stride) * width + x];
				float v11 = value[(y + stride) * width + (x + stride)];

				// Set the value of the current point to the average of the corners + a random value
				value[(y + half) * width + (x + half)] = (v00 + v10 + v01 + v11) / 4.0f + dist(rando) * range;
			}
		}

		// Square step - top / bottom edges
		for (unsigned x = half; x < limit_x; x += stride)
		{
			// Get the values for the points adjacent to the top edge
			float v_mid = value[half * width + x];
			float v_left = value[x - half];
			float v_right = value[x + half];

			// Set the value for the top edge at the current x coordinate to the average of the adjacent points + a random value
			value[x] = (v_mid + v_left + v_right) / 3.0f + dist(rando) * range;

			// Get the values for the points adjacent to the bottom edge
			v_mid = value[(limit_y - half) * width + x];
			v_left = value[limit_y * width + (x - half)];
			v_right = value[limit_y * width + (x + half)];

			// Set the value for the bottom edge at the current x coordinate to the average of the adjacent points + a random value
			value[limit_y * width + x] = (v_mid + v_left + v_right) / 3.0f + dist(rando) * range;
		}

		// Square step - left / right edges
		for (unsigned y = half; y < limit_y; y += stride)
		{
			// Get the values for the points adjacent to the left edge
			float v_mid = value[y * width + half];
			float v_top = value[(y - half) * width];
			float v_bottom = value[(y + half) * width];

			// Set the value for the left edge at the current y coordinate to the average of the adjacent points + a random value
			value[y * width] = (v_mid + v_top + v_bottom) / 3.0f + dist(rando) * range;

			// Get the values for the points adjacent to the right edge
			v_mid = value[y * width + (limit_x - half)];
			v_top = value[(y - half) * width + limit_x];
			v_bottom = value[(y + half) * width + limit_x];

			// Set the value for the right edge at the current y coordinate to the average of the adjacent points + a random value
			value[y * width + limit_x] = (v_mid + v_top + v_bottom) / 3.0f + dist(rando) * range;
		}

		// Square step - center points
		bool offset = true;
		for (unsigned y = half; y < limit_y; y += half)
		{
			for (unsigned x = offset ? stride : half; x < limit_x; x += stride)
			{
				// Get the values of the four grid points adjacent the current grid point
				float v_top = value[(y - half) * width + x];
				float v_bottom = value[(y + half) * width + x];
				float v_left = value[y * width + (x - half)];
				float v_right = value[y * width + (x + half)];

				// Set the value of the current point to the average of the adjacent points + a random value
				value[y * width + x] = (v_top + v_bottom + v_left + v_right) / 4.0f + dist(rando) * range;
			}

			offset = !offset;
		}

		stride /= 2;
	}
}

PlasmaNoise::PlasmaNoise(const PlasmaNoise& copy)
{
	width = copy.width;
	height = copy.height;
	value = new float[width * height];
	memcpy(value, copy.value, width * height * sizeof(float));
}

///
/// Random point noise
///

PointNoise::PointNoise(unsigned x_bias, unsigned y_bias, unsigned num_points, unsigned seed)
{
	width = x_bias;
	height = y_bias;
	array_size = width * height;

	// Generate points using random x and y values
	std::default_random_engine rando(seed);
	std::uniform_real_distribution<float> x_dist(0.0f, (float)width);
	std::uniform_real_distribution<float> y_dist(0.0f, (float)height);

	points = new std::vector<FVector2D>[array_size];
	for (unsigned i = 0; i < num_points; ++i)
	{
		FVector2D point(x_dist(rando), y_dist(rando));

		// Add the point to the point grid
		int X = (int)point.X;
		int Y = (int)point.Y;
		points[X + Y * width].push_back(point);
	}
}

PointNoise::PointNoise(const PointNoise& copy)
{
	width = copy.width;
	height = copy.height;
	array_size = copy.array_size;

	points = new std::vector<FVector2D>[array_size];
	for (unsigned i = 0; i < array_size; ++i)
	{
		for (unsigned j = 0; j < copy.points[i].size(); ++j)
		{
			points[i].push_back(copy.points[i][j]);
		}
	}
}

PointNoise::~PointNoise()
{
	if (points != nullptr)
	{
		delete[] points;
	}
}

void PointNoise::scale(unsigned sample_width, unsigned sample_height)
{
	scale_x = (float)width / sample_width;
	scale_y = (float)height / sample_height;
}

FVector2D PointNoise::getNearest(FVector2D location) const
{
	// Get the grid location to the top left of the current point
	int X = (int)location.X - 1;
	int Y = (int)location.Y - 1;

	// The closest point found so far
	FVector2D nearest(0.0f, 0.0f);
	// The distance to the closest point
	float nearest_distance = (float)width;

	// Check each grid cell surrounding the cell containing to current point
	for (unsigned y = 0; y < 3; ++y)
	{
		long offset = (Y + y) * width;
		for (unsigned x = 0; x < 3; ++x)
		{
			// Get the cell's location in the points array
			long cell = (X + x) + offset;
			if (cell > -1 && cell < (long)array_size)
			{
				// Check each point in the cell
				std::vector<FVector2D>* point_data = &points[cell];
				for (unsigned i = 0; i < point_data->size(); ++i)
				{
					// Check the distance to the point
					float dist = distance2D(location, point_data->at(i));
					if (dist < nearest_distance)
					{
						nearest_distance = dist;
						nearest = point_data->at(i);
					}
				}
			}
		}
	}

	return nearest;
}

float PointNoise::dot(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the nearest neighbor to the current point
	FVector2D loc(x, y);
	FVector2D nearest = getNearest(loc);

	// Calculate the noise value based on distance
	float value = 1.0f - sqrt(distance2D(loc, nearest)) * 4;

	return std::max(-1.0f, value);
}

float PointNoise::worley(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the nearest neighbor to the current point
	FVector2D loc(x, y);
	FVector2D nearest = getNearest(loc);

	// Calculate the noise value based on distance
	float value = sqrt(distance2D(loc, nearest)) * 2 - 1.0f;

	return std::min(1.0f, value);
}

GridNoise::GridNoise(unsigned _width, unsigned _height, unsigned seed)
{
	width = _width;
	height = _height;
	array_size = width * height;

	// Generate points at random locations within a unit grid
	std::default_random_engine rando(seed);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	points = new FVector2D[array_size];
	for (unsigned y = 0; y < height; ++y)
	{
		for (unsigned x = 0; x < width; ++x)
		{
			unsigned loc = x + y * width;
			points[loc].X = x + dist(rando);
			points[loc].Y = y + dist(rando);
		}
	}
}

GridNoise::GridNoise(const GridNoise& copy)
{
	width = copy.width;
	height = copy.height;
	array_size = copy.array_size;
	points = new FVector2D[array_size];
	memcpy(points, copy.points, array_size * sizeof(FVector2D));
}

GridNoise::~GridNoise()
{
	if (points != nullptr)
	{
		delete[] points;
	}
}

inline FVector2D GridNoise::getPoint(unsigned x, unsigned y) const
{
	return points[x + y * width];
}

inline FVector2D GridNoise::getNearest(FVector2D location) const
{
	// Get the grid location to the top left of the current point
	int X = (int)location.X - 1;
	int Y = (int)location.Y - 1;

	// The closest point found so far
	FVector2D nearest(0.0f, 0.0f);
	// The distance to the closest point
	float nearest_distance = (float)width;

	// Check each grid cell surrounding the cell containing to current point
	for (unsigned y = 0; y < 3; ++y)
	{
		long offset = (Y + y) * width;
		for (unsigned x = 0; x < 3; ++x)
		{
			// Get the cell's location in the point array
			long cell = (X + x) + offset;
			if (cell > -1 && cell < (long)array_size)
			{
				// Check the distance to the point
				float dist = distance2D(location, points[cell]);
				if (dist < nearest_distance)
				{
					nearest_distance = dist;
					nearest = points[cell];
				}
			}
		}
	}

	return nearest;
}

float GridNoise::dot(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the nearest neighbor to the current point
	FVector2D loc(x, y);
	FVector2D nearest = getNearest(loc);

	// Calculate the noise value based on distance
	float value = 1.0f - sqrt(distance2D(loc, nearest)) * 4;

	return std::max(-1.0f, value);
}

float GridNoise::worley(float x, float y) const
{
	// Scale noise values
	x *= scale_x;
	y *= scale_y;

	// Get the nearest neighbor to the current point
	FVector2D loc(x, y);
	FVector2D nearest = getNearest(loc);

	// Calculate the noise value based on distance
	float value = distance2D(loc, nearest) - 1.0f;

	return std::min(1.0f, value);
}