#pragma once
#include "raylib.h"

// Basically.. i had a perlin class but then multithreaded so i couldnt have the threadingtask be a member var so this is the remains.. 

void GenFinalImg(int width, int height, int id, Image& FinalImg, Color* Data);
void GenPerlinNoiseThreaded(int width, int height, float frequency, float amplitude, int octaves, float scale, unsigned char seed, int ID, int threads, Image& Img);
