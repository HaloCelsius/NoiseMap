#define STB_PERLIN_IMPLEMENTATION
#include <cstdlib>
#include <iostream>
#include <cmath>
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

void GenPerlinNoiseThreaded(int totalWidth, int totalHeight, float frequency, float amplitude, int octaves, float scale, unsigned char seed, int ID, int threads, Image& Img)
{
    int WidthS = totalWidth;
    int HeightS = totalHeight / threads;
    int startYGlobal = ID * HeightS;

    Color* Data = (Color*)RL_MALLOC(HeightS * WidthS * sizeof(Color));
    float totalAspectRatio = static_cast<float>(totalWidth) / static_cast<float>(totalHeight);

    //std::cout << "Thread " << ID << " processing rows " << startYGlobal << " to " << startYGlobal + HeightS - 1 << std::endl;

    for (int y = 0; y < HeightS; y++)
    {
        int globalY = startYGlobal + y;

        for (int x = 0; x < WidthS; x++)
        {
            float nx = (float)x * (scale / (float)totalWidth);
            float ny = (float)globalY * (scale / (float)totalHeight);

            if (totalWidth > totalHeight) {
                nx *= totalAspectRatio;
            }
            else if (totalHeight > totalWidth) {
                ny *= (1.0f / totalAspectRatio);
            }

            float perlin = stb_perlin_fbm_noise3_new(nx, ny, 1.0f, frequency, amplitude, octaves, seed, ID + 1);

            if (perlin < -1.0f) perlin = -1.0f;
            if (perlin > 1.0f) perlin = 1.0f;

            float newPerlin = (perlin + 1.0f) / 2.0f;
            unsigned char intensity = static_cast<unsigned char>(newPerlin * 255.0f);

            Data[y * WidthS + x] = { intensity, intensity, intensity, 255 };
        }
    }

    GenFinalImg(WidthS, HeightS, ID, Img, Data);

    RL_FREE(Data);
}