#pragma once
#include <vector>
#include "raylib.h"
#include "PerlinNoise.h"

namespace std
{
	class thread;
}

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

	Image RadialGradiant;
	Color* alpha;
	void Generate(int Width, int Height, float scale, float frequency, float amplitude, int octaves, unsigned char seed, bool terain, float islandthresh, int threadcount);

	IslandGen(int Width, int Height);
	~IslandGen();

private:
	Color ReturnBiome(int clr);

	std::vector<Biome*> biomes_;
	std::vector<std::thread> VecThreads;

	bool SaveImg = false;
};