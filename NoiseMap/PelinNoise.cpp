#define STB_PERLIN_IMPLEMENTATION
#include <cstdlib>
#include <iostream>
#include <cmath>
#include "raylib.h"
#include "PerlinNoise.h"
#include "stb_perlin.h"

void GenFinalImg(int width, int height, int id, Image& FinalImg, Color* Data)
{
    int startY = height * id;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            ImageDrawPixel(&FinalImg, x, startY + y, Data[x + y * width]);
        }
    }
}

void GenPerlinNoiseThreaded(int totalWidth, int totalHeight, float frequency, float amplitude, int octaves, float scale, unsigned char seed, int ID, int threads, Image& Img)
{
    int sliceWidth = totalWidth;
    int sliceHeight = totalHeight / threads; 
    int startYGlobal = ID * sliceHeight;

    Color* Data = (Color*)RL_MALLOC(sliceWidth * sliceHeight * sizeof(Color));
    if (!Data) {
        std::cerr << "Error: Failed to allocate memory for thread " << ID << std::endl;
        return;
    }
    float totalAspectRatio = static_cast<float>(totalWidth) / static_cast<float>(totalHeight);

    std::cout << "Thread " << ID << " processing rows " << startYGlobal << " to " << startYGlobal + sliceHeight - 1 << std::endl;

    for (int y = 0; y < sliceHeight; y++) 
    {
        int globalY = startYGlobal + y;

        for (int x = 0; x < sliceWidth; x++) 
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

            float newPerlin = (perlin + 1.0f) / 2.0f; // Map from [-1, 1] to [0, 1]
            unsigned char intensity = static_cast<unsigned char>(newPerlin * 255.0f);

            Data[y * sliceWidth + x] = { intensity, intensity, intensity, 255 };
        }
    }

    GenFinalImg(sliceWidth, sliceHeight, ID, Img, Data);

    RL_FREE(Data); 
}
