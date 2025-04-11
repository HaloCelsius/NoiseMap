#include "IslandGen.h"
#include "PerlinNoise.h"
#include <thread>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

#include <iostream>
#include "other/zlib/zlib.h"
#include "other/base64.h"
#include "other/json.hpp"

IslandGen::IslandGen(int Width, int Height)
{
	RadialGradiant = GenImageGradientRadial(Width, Height, 0.5, { 0,0,0,255 }, { 255,255,255,0 });
	alpha = LoadImageColors(RadialGradiant);

	biomes_.push_back(new Biome{ { 39,146,240, 255 }, 0.2 }); // Deep Water
	biomes_.push_back(new Biome{ { 80,218,254, 255 }, 0.4 }); // Shallow Water
	biomes_.push_back(new Biome{ { 250, 234, 99, 255 }, 0.5 }); // Sand
	biomes_.push_back(new Biome{ { 171, 219, 35, 255 }, 0.7 }); // Grass
	biomes_.push_back(new Biome{ { 34, 139, 34, 255 }, 0.9 }); // Forest
	biomes_.push_back(new Biome{ { 59,59,59, 255 }, 1.0 }); // Rock

	generatedWidth = 0;
	generatedHeight = 0;
}

IslandGen::~IslandGen()
{
	for (auto biome : biomes_)
	{
		delete biome;
	}
	UnloadImage(RadialGradiant);
	UnloadImageColors(alpha);
	UnloadTexture(FinalTexture2D);
	//UnloadImage(FinalImage3D); 
}

void IslandGen::Generate(int Width, int Height, float scale, float frequency, float amplitude, int octaves, unsigned char seed, bool terain, float islandthresh, int threadcount)
{
    UnloadImage(FinalImage3D); 
    UnloadTexture(FinalTexture2D);
    terrainMapColors.clear(); 

    generatedWidth = Width;
    generatedHeight = Height;
    terrainMapColors.resize(Width * Height);


    Image PerlinNoise = GenImageColor(Width, Height, WHITE); 

    VecThreads.clear();
    for (int i = 0; i < threadcount; i++)
    {
        VecThreads.emplace_back(GenPerlinNoiseThreaded, Width, Height, frequency, amplitude, octaves, scale, seed, i, threadcount, std::ref(PerlinNoise));
    }
    for (int i = 0; i < VecThreads.size(); i++)
    {
        VecThreads[i].join();
    }

    Color* PerlinColour = LoadImageColors(PerlinNoise);
    Image FinalPicture2D = GenImageColor(Width, Height, WHITE); 
    FinalImage3D = GenImageColor(Width, Height, RED);

    for (int y = 0; y < PerlinNoise.height; y++)
    {
        for (int x = 0; x < PerlinNoise.width; x++)
        {
            int index = (y * PerlinNoise.width) + x;

            int clr = ((PerlinColour[index].r + PerlinColour[index].g + PerlinColour[index].b) / 3);
            if (!terain) {
                clr = static_cast<int>(clr * (static_cast<float>(alpha[index].a) / islandthresh));
            }


            if (clr > 255) clr = 255;
            if (clr < 0) clr = 0;

            Color BiomeColor = ReturnBiome(clr);
            terrainMapColors[index] = BiomeColor;

            ImageDrawPixel(&FinalPicture2D, x, y, BiomeColor);

            Color HeightColor = { static_cast<unsigned char>(clr) , static_cast<unsigned char>(clr), static_cast<unsigned char>(clr), 255 };
            ImageDrawPixel(&this->FinalImage3D, x, y, HeightColor);
        }
    }

    if (SaveImg) {
        ExportImage(FinalImage3D, "HeightMap.png");
        ExportImage(PerlinNoise, "PerlinNoise.png");
        ExportImage(FinalPicture2D, "TerrainMap.png");
    }

    UnloadImageColors(PerlinColour);
    FinalTexture2D = LoadTextureFromImage(FinalPicture2D);

    UnloadImage(FinalPicture2D);
    UnloadImage(PerlinNoise);
}


void IslandGen::ExportToJson(const std::string& filename, const std::map<Color, std::string>& biomeToGroundIdMap)
{
    if (generatedWidth == 0 || generatedHeight == 0 || terrainMapColors.empty()) {
        TraceLog(LOG_WARNING, "Island data not generated yet. Cannot export.");
        throw std::runtime_error("Island data not generated. Call Generate() first.");
        return;
    }

    std::vector<JsonLoc> dictionary;
    std::unordered_map<JsonLoc, short, JsonLoc::Hasher> dictionaryIndexMap;
    std::vector<unsigned char> dataBuffer;
    dataBuffer.reserve(generatedWidth * generatedHeight * 2); // 2 bytes per tile index

    for (int y = 0; y < generatedHeight; ++y) {
        for (int x = 0; x < generatedWidth; ++x) {
            int index = y * generatedWidth + x;
            Color biomeColor = terrainMapColors[index];

            std::string groundId = "Unknown"; 
            auto mapIt = biomeToGroundIdMap.find(biomeColor);
            if (mapIt != biomeToGroundIdMap.end()) {
                groundId = mapIt->second;
            }
            else {
                TraceLog(LOG_WARNING, "Missing biome mapping for color: R%d G%d B%d", biomeColor.r, biomeColor.g, biomeColor.b);
            }

            JsonLoc currentLoc = { groundId, {}, {} }; 

            short dictIndex;
            auto dictIt = dictionaryIndexMap.find(currentLoc);
            if (dictIt == dictionaryIndexMap.end()) {
                dictIndex = static_cast<short>(dictionary.size());
                dictionary.push_back(currentLoc);
                dictionaryIndexMap[currentLoc] = dictIndex;
            }
            else {
                dictIndex = dictIt->second;
            }

            dataBuffer.push_back(static_cast<unsigned char>((dictIndex >> 8) & 0xFF)); // High byte
            dataBuffer.push_back(static_cast<unsigned char>(dictIndex & 0xFF));       // Low byte
        }
    }

    uLongf compressedBufferSize = compressBound(dataBuffer.size());
    std::vector<unsigned char> compressedData(compressedBufferSize);
    int zlibResult = compress(compressedData.data(), &compressedBufferSize, dataBuffer.data(), dataBuffer.size());

    if (zlibResult != Z_OK) {
        throw std::runtime_error("Zlib compression failed with error code: " + std::to_string(zlibResult));
    }
    compressedData.resize(compressedBufferSize); 
    std::string base64Data = base64_encode(compressedData.data(), compressedData.size());

    nlohmann::json j;
    j["width"] = generatedWidth;
    j["height"] = generatedHeight;
    j["data"] = base64Data;

    nlohmann::json dict_json = nlohmann::json::array();
    for (const auto& loc : dictionary) {
        nlohmann::json loc_j;
        loc_j["ground"] = loc.ground.empty() ? nullptr : nlohmann::json(loc.ground); 

        if (!loc.objs.empty()) {
            loc_j["objs"] = nlohmann::json::array();
            for (const auto& obj : loc.objs) {
                nlohmann::json obj_j;
                obj_j["id"] = obj.id;
                if (!obj.name.empty()) obj_j["name"] = obj.name; 
                loc_j["objs"].push_back(obj_j);
            }
        }
        else {
            loc_j["objs"] = nullptr;
        }

        // Add regions serialization (not usable yet tho)
        if (!loc.regions.empty()) {
            loc_j["regions"] = nlohmann::json::array();
            for (const auto& reg : loc.regions) {
                nlohmann::json reg_j;
                reg_j["id"] = reg.id;
                if (!reg.name.empty()) reg_j["name"] = reg.name;
                loc_j["regions"].push_back(reg_j);
            }
        }
        else {
            loc_j["regions"] = nullptr;
        }

        dict_json.push_back(loc_j);
    }
    j["dict"] = dict_json;
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    outFile << j.dump(); 
    outFile.close();
}

Color IslandGen::ReturnBiome(int clr)
{
	float normalizedClr = clr / 255.0f;
	if (normalizedClr > 1.0f) { normalizedClr = 1.0f; }
	if (normalizedClr < 0.0f) { normalizedClr = 0.0f; }

	for (const auto& biome : this->biomes_)
	{
		if (normalizedClr <= biome->threshold)
		{
			return biome->color;
		}
	}

    return BLACK;
}