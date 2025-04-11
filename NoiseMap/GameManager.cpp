#include "GameManager.h"
#include "IslandGen.h"

GameManager::GameManager()
{
	this->WindowSize.x = WindowWidth;
	this->WindowSize.y = WindowHeight;
}

GameManager::~GameManager()
{
	delete Island;
	UnloadModel(model);
}

void GameManager::Run()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(WindowSize.x, WindowSize.y, WindowName);
	SetTargetFPS(TargetFPS);
	camera = { 0 };
	camera.position = { 18.0f, 25.0f, 18.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	Island = new IslandGen(WindowWidth, WindowHeight);

	while (!WindowShouldClose())
	{
		Update();
		Draw();
	}

	CloseWindow();
}

void GameManager::Update()
{
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
	{
		unsigned char seed = GetRandomValue(0, 255);

		Island->Generate
		(
			GetScreenWidth(), 
			GetScreenHeight(), 
			IslandScale, 
			IslandFrequency, 
			IslandAmplitude, 
			IslandOctaves, 
			seed, 
			Terrain, 
			IslandThreshold,
			Threads
		);

		FinalRender = Island->FinalTexture2D;

		// 3D Noise-map
		UnloadMesh(mesh);

		mesh = GenMeshHeightmap(Island->FinalImage3D, { 20, 5, 20 });
		model = LoadModelFromMesh(mesh);
		model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = Island->FinalTexture2D;
	}

	if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
	{
		Mode3D = !Mode3D;
	}
}

void GameManager::Draw()
{
	if (Mode3D) { UpdateCamera(&camera, CAMERA_ORBITAL); }

	BeginDrawing();
	ClearBackground(RAYWHITE);

	if (Mode3D)
	{
		BeginMode3D(camera);
		DrawModel(model, { -10.0f, 0.0f, -10.0f }, 1.0f, WHITE);
		DrawGrid(22, 1.0f);
		EndMode3D();
	}
	else
	{
		if (FinalRender.id != 0)
		{
			DrawTexture(FinalRender, 0, 0, WHITE);
		}
	}

	EndDrawing();
}