#define STB_PERLIN_IMPLEMENTATION
#include <cstdlib>
#include "raylib.h"
#include "PerlinNoise.h"
#include "stb_perlin.h"

Image PerlinNoise::GenImagePerlinNoiseNew(int width, int height, int offsetx, int offsety, float frequency, float amplitude, int octaves, float scale, unsigned char seed)
{
	Color* pixels = (Color*)RL_MALLOC(width * height * sizeof(Color));

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nx = (float)(x + offsetx) * (scale / (float)width);
            float ny = (float)(y + offsety) * (scale / (float)height);

            if (width > height) { nx *= aspectRatio; }
            else { ny /= aspectRatio; }

            float perlin = stb_perlin_fbm_noise3_new(nx, ny, 1.0f, frequency, amplitude, octaves, seed);

            if (perlin < -1.0f) perlin = -1.0f;
            if (perlin > 1.0f) perlin = 1.0f;

            float newPerlin = (perlin + 1.0f) / 2.0f;
            unsigned char intesity = (int)(newPerlin * 255.0f);

            pixels[y * width + x] = { intesity ,intesity ,intesity ,255 };
        }
    }

    Image image =
    {
        image.data = pixels,
        image.width = width,
        image.height = height,
        image.mipmaps = 1,
        image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}
