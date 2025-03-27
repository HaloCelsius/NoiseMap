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

	void Generate(int Width, int Height, float scale, float frequency, float amplitude, int octaves, unsigned char seed, bool terain, int islandthresh);

	IslandGen(bool saveImg);
	~IslandGen();

private:
	PerlinNoise perlin;
	Color ReturnBiome(int clr);
	std::vector<Biome*> biomes_;

	bool SaveImg = false;
};