#include "Tile.h"

Tile::Tile()
{
}

Tile::Tile(int x, int y, Image* img)
{
	this->x = x;
	this->y = y;
	this->Img = *img;
}

Tile::~Tile()
{
}
