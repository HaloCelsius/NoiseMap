#pragma once
#include "raylib.h"
#include "IslandGen.h"

class GameManager
{
private:
	Vector2 WindowSize;
	Image icon = LoadImage("Icon.png");
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

	// Island Generation Params:
	int Threads = 1;
	bool SaveImg = false;
	bool Mode3D = false;

	bool Terrain = false;
	double IslandDensity = 0.5;
	double IslandScale = 4;
	double IslandFrequency = 2.0;
	double IslandAmplitude = 0.5;
	int IslandOctaves = 6;
	int IslandThreshold = 160;
	unsigned char Seed = 1;
};
