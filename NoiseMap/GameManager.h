#pragma once
#include "raylib.h"
#include "IslandGen.h"
#include <map>

class GameManager
{
private:
	Vector2 WindowSize;
	Image icon;
	const char* WindowName = "Island Generation";
	static const int TargetFPS = 240;

	Texture2D FinalRender;
	IslandGen* Island;

	Mesh mesh;
	Model model;
	Camera camera;

	const int WindowWidth = 512;
	const int WindowHeight = 512;

public:
	GameManager();
	~GameManager();

	void Run();
	void Update();
	void Draw();
	void GenerateIsland();
	// Island Generation Params:

	int Threads = 8;
	bool SaveImg = true;
	bool Mode3D = false;

	bool Terrain = false;
	double IslandDensity = 0.5;
	double IslandScale = 3;
	double IslandFrequency = 2.0;
	double IslandAmplitude = 0.5;
	int IslandOctaves = 8;
	int IslandThreshold = 200;
	unsigned char Seed = 1;

	std::map<Color, std::string> biomeMap;
};
