#pragma once
#include <vector>

#include "raylib.h"

class GameManager
{
private:

	// Raylib Stuff
	Vector2 WindowSize;

	const char* WindowName = "Window Name";		// Name of Window

	static const int TargetFPS = 60;		// Target Frame Rate
	float deltaTime = 0.f;					// Time Last Frame took to render
	float fixedDelta;						// How many times to Update per Second
	float currentFixedCount = 0.f;			// Counter for FixedDelta

	const int Tiles = 10;


public:
	Color ReturnBiome(int clr);

	std::vector<Image*> map;

	GameManager();
	~GameManager();

	void Run();
	void Update();
	void Draw();


};
