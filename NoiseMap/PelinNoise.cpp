#define STB_PERLIN_IMPLEMENTATION
#include <cstdlib>
#include <iostream>

#include "raylib.h"
#include "PerlinNoise.h"
#include "stb_perlin.h"

void GenFinalImg(int width, int height, int id, Image& FinalImg, Color* Data)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			ImageDrawPixel(&FinalImg, x, y + (height * id), Data[x + y * width]);
		}
	}
}

void GenPerlinNoiseThreaded(int width, int height, float frequency, float amplitude, int octaves, float scale, unsigned char seed, int ID, int threads, Image& Img)
{
	int WidthS = width;
	int HeightS = height / threads;

	Color* Data = (Color*)RL_MALLOC(WidthS * HeightS * sizeof(Color));

	float aspectRatio = static_cast<float>(WidthS) / static_cast<float>(HeightS);
	std::cout << ID << std::endl;
	for (int y = 0; y < HeightS; y++)
	{
		for (int x = 0; x < WidthS; x++)
		{
			float nx = (float)x * (scale / (float)WidthS);
			float ny = (float)(y) * (scale / (float)HeightS);

			if (WidthS > HeightS) { nx *= aspectRatio; }
			//else { ny /= aspectRatio; }

			float perlin = stb_perlin_fbm_noise3_new(nx, ny, 1.0f, frequency, amplitude, octaves, seed, ID+1);

			if (perlin < -1.0f) perlin = -1.0f;
			if (perlin > 1.0f) perlin = 1.0f;

			float newPerlin = (perlin + 1.0f) / 2.0f;
			unsigned char intesity = (int)(newPerlin * 255.0f);

			Data[((y * WidthS + x))] = { intesity ,intesity ,intesity ,255 };
		}
	}
	GenFinalImg(WidthS, HeightS, ID, Img, Data);
	UnloadImageColors(Data);
}
