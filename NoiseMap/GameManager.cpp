#include "GameManager.h"
#include "Tile.h"

GameManager::GameManager()
{
	this->WindowSize.x = 700;
	this->WindowSize.y = 700;
	fixedDelta = 1.f / TargetFPS;
}

GameManager::~GameManager()
{

}

void GameManager::Run()
{
	InitWindow(WindowSize.x, WindowSize.y, WindowName);
	SetTargetFPS(TargetFPS);

	Image* PerinNoise = new Image{ GenImagePerlinNoise(WindowSize.x, WindowSize.y, 0, 0, 3) };
	Image* ScreenImage2 = new Image{ GenImageGradientRadial(WindowSize.x, WindowSize.y,0.5, {0,0,0,255},{255,255,255,0})};

	// Texture Data

	Color* colors = LoadImageColors(*PerinNoise);
	Color* colors2 = LoadImageColors(*ScreenImage2);
	Tile Top[70];

	for (int i = 0; i < 70; i++)
	{
		Top[i].Img = GenImageColor(100, 100, RED);
	}

	BeginDrawing();
	ClearBackground(RAYWHITE);

	for (int y = 0; y < WindowSize.y; y ++)
	{
		for (int x = 0; x < WindowSize.x; x++)
		{
			int index = (y * WindowSize.x) + x;

			Color NoisePixel = colors[index];
			float aValue = (float)colors2[index].a / 255.0f;
			 int clr = ((NoisePixel.r + NoisePixel.g + NoisePixel.b) / 3) * aValue;

			Color MapPixel = ReturnBiome(clr);

			ImageDrawPixel(&Top[(x/100) + ((y/100)*7)].Img, x % 100, y % 100, MapPixel);
		}
	}

	for (int i = 0; i < 70; i ++)
	{
		ImageBlurGaussian(&Top[i].Img, 100);
		Texture2D texture = LoadTextureFromImage(Top[i].Img);
		DrawTexture(texture, (i%7)*100, (i / 7) * 100, WHITE);
	}

	EndDrawing();

	while (!WindowShouldClose())
	{
		//Update();
		//Draw();
	}

	CloseWindow();
}

void GameManager::Update()
{
	
}

void GameManager::Draw()
{
	BeginDrawing();
	ClearBackground(RAYWHITE);;

	EndDrawing();
}

Color GameManager::ReturnBiome(int clr)
{
	unsigned char colour = (int)clr / 51;

	switch (colour)
	{
		case 0:
			return { 39,146,240, 255 };
		case 1:
			return { 80,218,254, 255 };
		case 2:
			return { 250, 234, 99, 255 };
		case 3:
			return { 171, 219, 35, 255 };
		case 4:
			return { 34, 139, 34, 255 };
		case 5:
			return { 59,59,59, 255 };
		default:
			return { 59,59,59, 255 };
	}
}