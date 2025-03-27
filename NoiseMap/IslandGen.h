#pragma once
#include <vector>
#include "raylib.h"
#include "PerlinNoise.h"

struct Biome
{
	Color color;
	float threshold;
};

class IslandGen
{
public:	
	Texture2D FinalTexture2D;
	Image FinalImage3D;

	void Generate(int Width, int Height, float scale, float frequency, float amplitude, int octaves, unsigned char seed);
	void Visualise3D();

	IslandGen();
	~IslandGen();

private:
	PerlinNoise perlin;
	Color ReturnBiome(int clr);
	std::vector<Biome*> biomes_;
};