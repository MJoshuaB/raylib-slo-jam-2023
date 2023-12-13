/*******************************************************************************************
 *
 *   raylib gamejam template
 *
 *   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
 *
 *   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2022-2023 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS       // Force custom modal dialogs usage
#include <emscripten/emscripten.h> // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h> // Required for: printf()
// #include <stdlib.h> // Required for:
// #include <string.h> // Required for:

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define MAX_ASTEROIDS 16
#define MAX_BULLETS 16

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum
{
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING
} GameScreen;

typedef struct
{
    Vector2 start;
    Vector2 end;
} Line;

typedef struct
{
    Line *lines;
    int lineCount;
} Shape;

typedef struct
{
    Vector2 pos;
    Vector2 acc;
    Vector2 vel;
    float angle;
} Player;

typedef struct
{
    Vector2 pos;
    Vector2 vel;
    float angle;
    float rotRate;
} Asteroid;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 1280;
static const int screenHeight = 720;
static const Vector2 ship[] = {{0, -30}, {-15, 15}, {0, 0}, {15, 15}}; // shape for the ship

static RenderTexture2D target = {0}; // Render texture to render our game

static Player player;
static Asteroid asteroids[MAX_ASTEROIDS];
static Camera2D camera;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void); // Update and Draw one frame

static void InitPlayer();
static void InitAsteroids();

static void UpdatePlayer();
static void UpdateAsteroids();

static void DrawShape(const Vector2 *points, int count, Vector2 position, float rotation);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE); // Disable raylib trace log messsages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");

    // TODO: Load resources / Initialize variables at this point
    InitPlayer();
    InitAsteroids();
    camera.target = (Vector2){player.pos.x, player.pos.y};
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60); // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);

    // TODO: Unload all loaded resources at this point

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    //----------------------------------------------------------------------------------

    UpdatePlayer();
    UpdateAsteroids();

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture,
    // it could be useful for scaling or further sahder postprocessing
    BeginTextureMode(target);
    BeginMode2D(camera);
    ClearBackground(BLACK);

    for (int y = 0; y < screenHeight; y += 50)
    {
        for (int x = 0; x < screenWidth; x += 50)
        {
            DrawPixel(x, y, WHITE);
        }
    }

    DrawShape(ship, 4, player.pos, player.angle);
    // TODO: Draw your game screen here
    // DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, SKYBLUE);

    EndMode2D();
    EndTextureMode();

    // Render to screen (main framebuffer)
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw render texture to screen, scaled if required
    DrawTexturePro(target.texture, (Rectangle){0, 0, (float)target.texture.width, -(float)target.texture.height}, (Rectangle){0, 0, (float)target.texture.width, (float)target.texture.height}, (Vector2){0, 0}, 0.0f, WHITE);

    // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
    //----------------------------------------------------------------------------------
}

static void InitPlayer()
{
    // Set up the player object
    player.pos = (Vector2){screenWidth / 2, screenHeight / 2};
    player.acc = (Vector2){0.0f, 0.0f};
    player.vel = (Vector2){0.0f, 0.0f};
    player.angle = 0.0f;
}
static void InitAsteroids()
{
    // Create the first n(4?) asteroids to start the game.
    for (int i = 0; i < 4; i++)
    {
        asteroids[i].pos = (Vector2){screenWidth / 2, screenHeight / 2};
        asteroids[i].vel = Vector2Rotate((Vector2){0.0f, -1.0f}, GetRandomValue(0, 314) / (2.0f * PI));
        asteroids[i].angle = GetRandomValue(0, 6283) / (2000.0f * PI);
        asteroids[i].rotRate = GetRandomValue(0, 62) / (2000.0f * PI) - PI;
    }
}

static void UpdatePlayer()
{
    if (IsKeyDown(KEY_W))
    {
        player.acc = Vector2Add(player.acc, Vector2Rotate((Vector2){0.0f, -20.f}, player.angle));
    }
    if (IsKeyDown(KEY_A))
    {
        player.angle -= 0.1f;
    }
    if (IsKeyDown(KEY_D))
    {
        player.angle += 0.1f;
    }
    player.vel = Vector2Add(player.vel, player.acc);
    player.vel = Vector2Scale(player.vel, 0.98f);
    player.pos = Vector2Add(player.pos, Vector2Scale(player.vel, GetFrameTime()));
    player.acc = (Vector2){0.0f, 0.0f};
    camera.target = Vector2Lerp(camera.target, player.pos, 0.1f);
}

static void UpdateAsteroids()
{
    for (int i = 0; i < MAX_ASTEROIDS; i++)
    {
        asteroids[i].pos = Vector2Add(asteroids[i].pos, asteroids[i].vel);
        asteroids[i].angle += asteroids[i].rotRate;
    }
}

static void DrawShape(const Vector2 *points, int count, Vector2 position, float rotation)
{
    float px = position.x, py = position.y;
    Vector2 start, end;
    for (int i = 0; i < count; i++)
    {
        start = Vector2Rotate(points[i], rotation);
        end = Vector2Rotate(points[(i + 1) % count], rotation);

        DrawLine(px + start.x, py + start.y, px + end.x, py + end.y, WHITE);
    }
}