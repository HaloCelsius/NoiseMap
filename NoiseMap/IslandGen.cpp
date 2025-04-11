#include "IslandGen.h"
#include "PerlinNoise.h"
#include <thread>

IslandGen::IslandGen(int Width, int Height)
{
	RadialGradiant = GenImageGradientRadial(Width, Height, 0.5, { 0,0,0,255 }, { 255,255,255,0 });
	alpha = LoadImageColors(RadialGradiant);

	// Biomes below, Thresholds arent really thresholds..
	biomes_.push_back(new Biome{ { 39,146,240, 255 }, 0.2 });
	biomes_.push_back(new Biome{ { 80,218,254, 255 }, 0.4 });
	biomes_.push_back(new Biome{ { 250, 234, 99, 255 }, 0.5 });
	biomes_.push_back(new Biome{ { 171, 219, 35, 255 }, 0.7 });
	biomes_.push_back(new Biome{ { 34, 139, 34, 255 }, 0.9 });
	biomes_.push_back(new Biome{ { 59,59,59, 255 }, 1.0 });
}

IslandGen::~IslandGen()
{
	for (auto biome: biomes_)
	{
		delete biome;
	}
	UnloadImage(RadialGradiant);
	UnloadImageColors(alpha);
	UnloadTexture(FinalTexture2D);
}

void IslandGen::Generate(int Width, int Height, float scale, float frequency, float amplitude, int octaves, unsigned char seed, bool terain, float islandthresh, int threadcount)
{
	UnloadImage(FinalImage3D);

	//Image PerinNoise = perlin.GenImagePerlinNoiseNew(Width, Height, 0, 0, frequency, amplitude, octaves, scale, seed);
	Image PerinNoise = GenImageColor(Width, Height, WHITE);

	VecThreads.clear();
	for (int i = 0; i < threadcount; i++)
	{
		VecThreads.emplace_back(GenPerlinNoiseThreaded, Width, Height, frequency, amplitude, octaves, scale, seed, i, threadcount, std::ref(PerinNoise));
	}

	for (int i = 0; i < VecThreads.size(); i++)
	{
		VecThreads[i].join();
	}

	Color* PerlinColour = LoadImageColors(PerinNoise);
	Image FinalPicture2D = GenImageColor(Width, Height, WHITE);
	FinalImage3D = GenImageColor(Width, Height, RED);

	for (int y = 0; y < PerinNoise.height; y++)
	{
		for (int x = 0; x < PerinNoise.width; x++)
		{
			int index = (y * PerinNoise.width) + x;

			int clr = ((PerlinColour[index].r + PerlinColour[index].g + PerlinColour[index].b) / 3) * (terain ? 1 : (float)alpha[index].a/islandthresh);

			if (clr > 255.0f) { clr = 255.0f; }
			if (clr < 0.0f) { clr = 0.0f; }

			Color Terain2D = ReturnBiome(clr);
			Color Perlin2D = { static_cast<unsigned char>(clr) , static_cast<unsigned char>(clr), static_cast<unsigned char>(clr), 255};

			ImageDrawPixel(&FinalPicture2D, x, y, Terain2D);
			ImageDrawPixel(&this->FinalImage3D, x, y, Perlin2D);
		}
	}

	if (SaveImg) {
		ExportImage(FinalImage3D, "HeightMap.png");
		ExportImage(PerinNoise, "PerlinNoise.png");
		ExportImage(FinalPicture2D, "TerrainMap.png");
	}

	UnloadTexture(FinalTexture2D);
	UnloadImageColors(PerlinColour);

	FinalTexture2D = LoadTextureFromImage(FinalPicture2D);

	UnloadImage(FinalPicture2D);
	UnloadImage(PerinNoise);
}

Color IslandGen::ReturnBiome(int clr)
{
	float colour = clr / static_cast<float>(255);
	if (colour > 1.0f) { colour = 1.0f; }
	if (colour < 0.0f) { colour = 0.0f; }

	for (auto thresh: this->biomes_)
	{
		if (colour <= thresh->threshold)
		{
			return thresh->color;
		}
	}

	return BLACK;
}