#pragma once
#include "raylib.h"
#include <vector>
#include <string>
#include <thread>
#include <map>

struct JsonObj {
	std::string id;
	std::string name;

	bool operator<(const JsonObj& other) const {
		if (id != other.id) return id < other.id;
		return name < other.name;
	}
	bool operator==(const JsonObj& other) const {
		return id == other.id && name == other.name;
	}
};

struct JsonLoc {
    std::string ground;
    std::vector<JsonObj> objs;
    std::vector<JsonObj> regions;

    bool operator==(const JsonLoc& other) const {
        return ground == other.ground && objs == other.objs && regions == other.regions;
    }
    struct Hasher {
        std::size_t operator()(const JsonLoc& k) const {
            using std::size_t;
            using std::hash;
            using std::string;
            using std::vector;

            size_t res = hash<string>()(k.ground);
            for (const auto& obj : k.objs) {
                res ^= (hash<string>()(obj.id) << 1) ^ (hash<string>()(obj.name) << 2);
            }
            for (const auto& reg : k.regions) {
                res ^= (hash<string>()(reg.id) << 3) ^ (hash<string>()(reg.name) << 4);
            }
            return res;
        }
    };
};

struct Biome {
    Color color;
    float threshold;
};

enum class BiomeType {
    DEEP_WATER,
    SHALLOW_WATER,
    SAND,
    DARK_SAND,
    GRASS,
    FOREST,
    ROCK,
    SNOW,
    DESERT,
    RIVER,
    ROAD,
    UNKNOWN
};

const Color DEEP_WATER_COLOR = { 39, 146, 240, 255 };
const Color SHALLOW_WATER_COLOR = { 80, 218, 254, 255 };
const Color SAND_COLOR = { 250, 234, 99, 255 };
const Color DARK_SAND_COLOR = { 210, 194, 79, 255 };
const Color GRASS_COLOR = { 171, 219, 35, 255 };
const Color FOREST_COLOR = { 34, 139, 34, 255 };
const Color ROCK_COLOR = { 59, 59, 59, 255 };
const Color ROAD_COLOR = { 100, 100, 100, 255 };

class IslandGen
{
public:
    IslandGen(int Width, int Height);
    ~IslandGen();

    Texture2D FinalTexture2D;
    Image FinalImage3D;

    void Generate(int Width, int Height, float scale, float frequency, float amplitude, int octaves, unsigned char seed, bool terain, float islandthresh, int threadcount);
    void ExportToJson(const std::string& filename, const std::map<Color, std::string>& biomeToGroundIdMap);

private:
    Image RadialGradiant;
    Color* alpha;

    std::vector<std::thread> VecThreads;
    std::vector<Biome*> biomes;

    Color ReturnBiome(int clr); 

    int generatedWidth = 0;
    int generatedHeight = 0;
    std::vector<Color> terrainMapColors;

    bool SaveImg = true;

    bool AreColorsEqual(Color c1, Color c2);
};

inline bool operator<(const Color& lhs, const Color& rhs) {
    if (lhs.r != rhs.r) return lhs.r < rhs.r;
    if (lhs.g != rhs.g) return lhs.g < rhs.g;
    if (lhs.b != rhs.b) return lhs.b < rhs.b;
    return lhs.a < rhs.a;
}