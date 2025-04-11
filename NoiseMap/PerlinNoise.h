#pragma once
#include "raylib.h"

void GenFinalImg(int width, int height, int id, Image& FinalImg, Color* Data);
void GenPerlinNoiseThreaded(int width, int height, float frequency, float amplitude, int octaves, float scale, unsigned char seed, int ID, int threads, Image& Img);
