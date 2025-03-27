#pragma once
#include <vector>
#include "raylib.h"
#include "IslandGen.h"

class GameManager
{
private:
	Vector2 WindowSize;

	const char* WindowName = "Island";		// Name of Window

	static const int TargetFPS = 60;		// Target Frame Rate

	float deltaTime = 0.f;					// Time Last Frame took to render
	float fixedDelta;						// How many times to Update per Second
	float currentFixedCount = 0.f;			// Counter for FixedDelta

	bool generateNew = true;
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

	// Island Gen Params:
	double IslandDensity = 0.5;
	double IslandScale = 3;
	double IslandFrequency = 2.0;
	double IslandAmplitude = 0.5;
	int IslandOctaves = 6;
	int Seed = 1;
};
