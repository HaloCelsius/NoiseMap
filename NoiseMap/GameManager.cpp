#include "GameManager.h"
#include "IslandGen.h"
#include <map>


//std::map<BiomeType, std::string> GameManager::CreateBiomeToServerIdMap() {
//    std::map<BiomeType, std::string> mapping;
//    mapping[BiomeType::DEEP_WATER] = "Deep Water"; 
//    mapping[BiomeType::SHALLOW_WATER] = "Shallow Water";
//    mapping[BiomeType::SAND] = "Gold Sand"; 
//    mapping[BiomeType::GRASS] = "Grass";
//    mapping[BiomeType::FOREST] = "Yellow Grass"; 
//    mapping[BiomeType::ROCK] = "Rock";
//    mapping[BiomeType::SNOW] = "Snow"; 
//    mapping[BiomeType::DESERT] = "Desert Sand"; 
//    mapping[BiomeType::RIVER] = "Shallow Water"; 
//    mapping[BiomeType::ROAD] = "Stone Road"; 
//    return mapping;
//}

std::map<Color, std::string> CreateBiomeToServerIdMap() {
	std::map<Color, std::string> mapping;
	mapping[{ 39, 146, 240, 255 }] = "Dark Water"; 
	mapping[{ 80, 218, 254, 255 }] = "Shallow Water"; 
	mapping[{ 250, 234, 99, 255 }] = "Gold Sand"; 
	mapping[{ 171, 219, 35, 255 }] = "Grass"; 
	mapping[{ 34, 139, 34, 255 }] = "Yellow Grass"; 
	mapping[{ 59, 59, 59, 255 }] = "Rock"; 
	return mapping;
}

GameManager::GameManager()
{
	this->WindowSize.x = WindowWidth;
	this->WindowSize.y = WindowHeight;
	Island = nullptr;
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

	Island = new IslandGen(GetScreenWidth(), GetScreenHeight());

	biomeMap = CreateBiomeToServerIdMap();

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

        UnloadMesh(mesh); 
        mesh = GenMeshHeightmap(Island->FinalImage3D, { 20, 5, 20 });
        model = LoadModelFromMesh(mesh); 
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = Island->FinalTexture2D;
    }

    if (IsKeyDown(KEY_SPACE)) {
        try {
            Island->ExportToJson("generated_island.jm", biomeMap);
            TraceLog(LOG_INFO, "Exported island to generated_island.jm");
        }
        catch (const std::exception& e) {
            TraceLog(LOG_ERROR, "Failed to export island: %s", e.what());
        }
    }

    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
    {
        Mode3D = !Mode3D; 
    }

    if (Mode3D) { UpdateCamera(&camera, CAMERA_ORBITAL); } 
}

void GameManager::Draw() 
{
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
            DrawTexturePro(FinalRender,
                { 0, 0, (float)FinalRender.width, (float)FinalRender.height }, 
                { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, // Destination rect
                { 0, 0 }, 
                0.0f, 
                WHITE);
        }
    }
    DrawFPS(10, 10);
    EndDrawing();
}