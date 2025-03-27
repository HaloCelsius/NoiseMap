#pragma once
#include "raylib.h"

class PerlinNoise
{
public:
	Image GenImagePerlinNoiseNew(int width, int height, int offsetx, int offsety, float frequency, float amplitude, int octaves, float scale, unsigned char seed);
private:
};