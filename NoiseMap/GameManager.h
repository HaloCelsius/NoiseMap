#pragma once
#include "raylib.h"
#include "IslandGen.h"

class GameManager
{
private:
	Vector2 WindowSize;

	Image icon = LoadImage("Icon.png");
	const char* WindowName = "Island Generation";
	static const int TargetFPS = 60;
	float fixedDelta;

	bool Mode3D = false;

	Texture2D FinalRender;
	IslandGen* Island;

	Mesh mesh;
	Model model;
	Camera camera;

public:
	GameManager();
	~GameManager();

	void Run();
	void Update();
	void Draw();

	// Island Generation Params:
	bool SaveImg = false;

	bool Terrain = false;
	double IslandDensity = 0.5;
	double IslandScale = 4;
	double IslandFrequency = 2.0;
	double IslandAmplitude = 0.5;
	int IslandOctaves = 6;
	int IslandThreshold = 160;
	unsigned char Seed = 1;
};
