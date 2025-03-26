#pragma once
#include <raylib.h>

class Tile
{
public:

	Tile();
	Tile(int x, int y, Image* img);
	~Tile();

	int x = 0;
	int y = 0;
	Image Img;

private:
};
