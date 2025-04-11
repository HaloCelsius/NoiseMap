#pragma once
#include <raylib.h>
#include <string>
#include <vector>

class Map
{

public:
	std::string Name;
	std::string Description; // Simple description of the level over-view, displayed to the user before starting;
	unsigned char Difficulty; // Visual indication of the difficulty of the map. 1-10 

private:
	int MapWidth;
	int MapHeight;
	int Level;

	std::vector<Vector4*> Objects; // Objects format:

	// First Value = ID
	// 2nd Value = Rotation
	// 3rd Value = PositionX
	// 4th Value = PositionY


};
