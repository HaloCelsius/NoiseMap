#include "GameManager.h"
#include "IslandGen.h"
#include <map>
#include <stdexcept>

int main()
{
    GameManager* Game = new GameManager();
    Game->Run();

    delete Game;
    return 0;
}

std::map<Color, std::string> CreateBiomeToServerIdMap() {
    std::map<Color, std::string> mapping;
    mapping[DEEP_WATER_COLOR] = "Dark Water";
    mapping[SHALLOW_WATER_COLOR] = "Shallow Water";
    mapping[DARK_SAND_COLOR] = "Dark Sand";
    mapping[SAND_COLOR] = "Gold Sand";
    mapping[ROAD_COLOR] = "Stone Road";
    mapping[GRASS_COLOR] = "Grass";
    mapping[FOREST_COLOR] = "Yellow Grass";
    mapping[ROCK_COLOR] = "Rock";
    return mapping;
}

GameManager::GameManager()
{
	this->WindowSize.x = WindowWidth;
	this->WindowSize.y = WindowHeight;
	Island = nullptr;
    model = { 0 };
    mesh = { 0 };
    FinalRender = { 0 };
}

GameManager::~GameManager()
{
	delete Island;

	UnloadModel(model);
    UnloadTexture(FinalRender);
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

    GenerateIsland();

	while (!WindowShouldClose())
	{
		Update();
		Draw();
	}

	CloseWindow();
}

void GameManager::GenerateIsland()
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

    UnloadImage(icon);
    FinalRender = Island->FinalTexture2D;
    icon = LoadImageFromTexture(FinalRender);

    SetWindowIcon(icon);
    UnloadMesh(mesh);
    mesh = GenMeshHeightmap(Island->FinalImage3D, { 20, 5, 20 });
    model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = Island->FinalTexture2D;
}


void GameManager::Update()
{
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        GenerateIsland(); 
    }

    if (IsKeyPressed(KEY_SPACE)) {
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

    if (IsWindowResized()) {
        GenerateIsland();
    }
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
        else {
            DrawText("Generating Texture...", 10, 40, 20, GRAY);
        }
    }
    DrawFPS(10, 10);
    EndDrawing();
}