#include "IslandGen.h"
#include "PerlinNoise.h"
#include "stb_perlin.h"
#include <thread>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <random> 
#include <iostream>

#include "other/zlib/zlib.h"
#include "other/base64.h"
#include "other/json.hpp"

bool IslandGen::AreColorsEqual(Color c1, Color c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

IslandGen::IslandGen(int Width, int Height)
{
	RadialGradiant = GenImageGradientRadial(Width, Height, 0.5, { 0,0,0,255 }, { 255,255,255,0 });
	alpha = LoadImageColors(RadialGradiant);

    biomes.push_back(new Biome{ DEEP_WATER_COLOR, 0.2f });
    biomes.push_back(new Biome{ SHALLOW_WATER_COLOR, 0.35f });
    biomes.push_back(new Biome{ DARK_SAND_COLOR, 0.4f });
    biomes.push_back(new Biome{ SAND_COLOR, 0.45f });
    biomes.push_back(new Biome{ ROAD_COLOR, 0.55f });
    biomes.push_back(new Biome{ GRASS_COLOR, 0.7f });
    biomes.push_back(new Biome{ FOREST_COLOR, 0.9f });
    biomes.push_back(new Biome{ ROCK_COLOR, 1.0f });

    generatedWidth = 0;
    generatedHeight = 0;
}

IslandGen::~IslandGen()
{
	for (auto biome : biomes)
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

            Color baseBiomeColor = ReturnBiome(clr);
            Color finalBiomeColor = baseBiomeColor;
            if (AreColorsEqual(baseBiomeColor, SAND_COLOR)) {
                bool nearWater = false;
                int neighbors[] = { index - 1, index + 1, index - Width, index + Width };

                for (int neighborIndex : neighbors) {
                    if (neighborIndex >= 0 && neighborIndex < Width * Height) {
                        if (clr < (int)(biomes[2]->threshold * 255)) {
                            nearWater = true;
                            break;
                        }
                    }
                }
                if (nearWater) {
                    finalBiomeColor = DARK_SAND_COLOR;
                }
            }

            terrainMapColors[index] = finalBiomeColor;

            Color HeightColor = { static_cast<unsigned char>(clr) , static_cast<unsigned char>(clr), static_cast<unsigned char>(clr), 255 };
            ImageDrawPixel(&this->FinalImage3D, x, y, HeightColor);
        }
    }

    std::vector<Color> tempTerrainMap = terrainMapColors;
    for (int y = 1; y < Height - 1; ++y) {
        for (int x = 1; x < Width - 1; ++x) {
            int index = y * Width + x;
            Color currentTileColor = terrainMapColors[index];

            if (AreColorsEqual(currentTileColor, GRASS_COLOR) || AreColorsEqual(currentTileColor, SAND_COLOR)) {
                bool nearWater = false;
                int neighbors[] = { index - 1, index + 1, index - Width, index + Width };
                for (int neighborIndex : neighbors) {
                    Color neighborColor = terrainMapColors[neighborIndex];
                    if (AreColorsEqual(neighborColor, SHALLOW_WATER_COLOR) || AreColorsEqual(neighborColor, DEEP_WATER_COLOR)) {
                        nearWater = true;
                        break;
                    }
                }

                if (nearWater) {
                    tempTerrainMap[index] = ROAD_COLOR;
                }
            }
        }
    }

    terrainMapColors = tempTerrainMap; 
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            ImageDrawPixel(&FinalPicture2D, x, y, terrainMapColors[y * Width + x]);
        }
    }

    if (SaveImg) {
        ExportImage(FinalImage3D, "HeightMap.png");
        ExportImage(PerlinNoise, "PerlinNoise.png");
        ExportImage(FinalPicture2D, "TerrainMap.png");
    }

    UnloadImageColors(PerlinColour);
    UnloadImage(PerlinNoise);

    FinalTexture2D = LoadTextureFromImage(FinalPicture2D);
    UnloadImage(FinalPicture2D);
}


void IslandGen::ExportToJson(const std::string& filename, const std::map<Color, std::string>& biomeToGroundIdMap)
{
    if (generatedWidth == 0 || generatedHeight == 0 || terrainMapColors.empty()) {
        TraceLog(LOG_WARNING, "Island data not generated yet. Cannot export.");
        return;
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 99);

    std::vector<JsonLoc> dictionary;
    std::unordered_map<JsonLoc, short, JsonLoc::Hasher> dictionaryIndexMap;
    std::vector<unsigned char> dataBuffer;
    dataBuffer.reserve(generatedWidth * generatedHeight * 2); // Estimate 2 bytes per tile index

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

            if (AreColorsEqual(biomeColor, SAND_COLOR) || AreColorsEqual(biomeColor, DARK_SAND_COLOR)) {
                if (dist(rng) < 5) { // 5% chance to place a beach towel
                    currentLoc.objs.push_back({"beachtowel_1"});
                }
            }

            short dictIndex;
            auto dictIt = dictionaryIndexMap.find(currentLoc);
            if (dictIt == dictionaryIndexMap.end()) {
                dictIndex = static_cast<short>(dictionary.size());
                if (dictionary.size() > 32767) {
                    TraceLog(LOG_ERROR, "Dictionary size exceeds short limit!");
                }
                dictionary.push_back(currentLoc);
                dictionaryIndexMap[currentLoc] = dictIndex;
            }
            else {
                dictIndex = dictIt->second;
            }

            dataBuffer.push_back(static_cast<unsigned char>((dictIndex >> 8) & 0xFF)); // High byte
            dataBuffer.push_back(static_cast<unsigned char>(dictIndex & 0xFF));        // Low byte
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
    outFile << j.dump(4);
    outFile.close();
    TraceLog(LOG_INFO, "Successfully exported island data to %s", filename.c_str());
}

Color IslandGen::ReturnBiome(int clr)
{
    float normalizedClr = clr / 255.0f;
    if (normalizedClr > 1.0f) { normalizedClr = 1.0f; }
    if (normalizedClr < 0.0f) { normalizedClr = 0.0f; }

    for (const auto& biome : this->biomes)
    {
        if (AreColorsEqual(biome->color, DARK_SAND_COLOR) || AreColorsEqual(biome->color, ROAD_COLOR)) {
            continue;
        }

        if (normalizedClr <= biome->threshold)
        {
            return biome->color;
        }
    }

    TraceLog(LOG_WARNING, "No biome found for normalized height: %f", normalizedClr);
    if (!biomes.empty()) {
        Biome* highestBiome = nullptr;
        for (int i = biomes.size() - 1; i >= 0; --i) {
            if (!AreColorsEqual(biomes[i]->color, DARK_SAND_COLOR) && !AreColorsEqual(biomes[i]->color, ROAD_COLOR)) {
                highestBiome = biomes[i];
                break;
            }
        }
        if (highestBiome) return highestBiome->color;
    }

    return BLACK;
}