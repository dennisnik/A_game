#include "raylib.h"
#include "common.h"
#include "player.h"
#include "world.h"
#include "particles.h"
#include "assets.h"
#include <stdio.h>

int main(void) {
    InitWindow((int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, "Raylib 2D Platformer - Enhanced");
    SetTargetFPS(60);

    // Initialize asset subsystem and load any available custom textures from assets/
    GameAssets assets;
    AssetsInit(&assets);
    AssetsLoad(&assets);

    ParticleSystemInit();

    World world;
    WorldInit(&world);

    Player player;
    PlayerInit(&player, (Vector2){ 80.0f, 380.0f });

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.position.x, player.position.y };
    camera.offset = (Vector2){ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.62f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    bool showDebug = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f; // Prevent huge delta spikes on window drags

        // Controls
        if (IsKeyPressed(KEY_R)) {
            WorldInit(&world);
            PlayerInit(&player, (Vector2){ 80.0f, 380.0f });
        }
        if (IsKeyPressed(KEY_F3)) {
            showDebug = !showDebug;
        }

        // Updates
        PlayerUpdate(&player, &world, dt);
        WorldUpdate(&world, &player, dt);
        ParticleSystemUpdate(dt);

        // Smooth Camera Follow with clamping within world boundaries
        float targetCamX = player.position.x + player.size.x * 0.5f;
        float targetCamY = player.position.y + player.size.y * 0.5f;

        float minCamX = SCREEN_WIDTH * 0.5f;
        float maxCamX = WORLD_WIDTH - SCREEN_WIDTH * 0.5f;
        if (targetCamX < minCamX) targetCamX = minCamX;
        if (targetCamX > maxCamX) targetCamX = maxCamX;

        float minCamY = SCREEN_HEIGHT * 0.5f;
        float maxCamY = WORLD_HEIGHT - SCREEN_HEIGHT * 0.4f;
        if (targetCamY < minCamY) targetCamY = minCamY;
        if (targetCamY > maxCamY) targetCamY = maxCamY;

        camera.target.x += (targetCamX - camera.target.x) * 6.5f * dt;
        camera.target.y += (targetCamY - camera.target.y) * 4.5f * dt;

        // Render
        BeginDrawing();
        ClearBackground((Color){ 16, 24, 40, 255 });

        // World (Camera space)
        BeginMode2D(camera);
            WorldDrawBackground(&world, camera);
            WorldDrawForeground(&world, &assets);
            ParticleSystemDraw();
            PlayerDraw(&player, &assets);
        EndMode2D();

        // UI / HUD (Screen space)
        // Top banner background
        DrawRectangle(0, 0, (int)SCREEN_WIDTH, 48, (Color){ 10, 15, 26, 180 });
        DrawLine(0, 48, (int)SCREEN_WIDTH, 48, (Color){ 52, 73, 94, 120 });

        // HUD Text
        DrawText(TextFormat("SCORE: %04d", player.score), 20, 15, 20, RAYWHITE);
        DrawCircle(195, 25, 7.0f, (Color){ 241, 196, 15, 255 });
        DrawText(TextFormat("%d / %d GEMS", player.score / 100, world.totalCoins), 210, 16, 17, (Color){ 241, 196, 15, 255 });

        // Bottom control guide
        DrawText("[A/D or Arrows] Move   |   [Space/W] Jump (Variable)   |   [R] Restart   |   [F3] Debug",
                 20, (int)SCREEN_HEIGHT - 25, 13, (Color){ 189, 195, 199, 210 });

        // Goal Reached Banner
        if (player.reachedGoal) {
            DrawRectangle(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, (Color){ 0, 0, 0, 140 });
            DrawRectangleRounded((Rectangle){ SCREEN_WIDTH * 0.5f - 220, SCREEN_HEIGHT * 0.5f - 90, 440, 180 }, 0.2f, 6, (Color){ 24, 34, 56, 240 });
            DrawRectangleRoundedLines((Rectangle){ SCREEN_WIDTH * 0.5f - 220, SCREEN_HEIGHT * 0.5f - 90, 440, 180 }, 0.2f, 6, (Color){ 46, 204, 113, 255 });

            DrawText("STAGE CLEAR!", (int)(SCREEN_WIDTH * 0.5f - 120), (int)(SCREEN_HEIGHT * 0.5f - 60), 34, (Color){ 46, 204, 113, 255 });
            DrawText(TextFormat("Final Score: %d", player.score), (int)(SCREEN_WIDTH * 0.5f - 75), (int)(SCREEN_HEIGHT * 0.5f - 15), 20, RAYWHITE);
            DrawText("Press [R] to Play Again", (int)(SCREEN_WIDTH * 0.5f - 95), (int)(SCREEN_HEIGHT * 0.5f + 30), 16, (Color){ 241, 196, 15, 255 });
        }

        // Debug info HUD
        if (showDebug) {
            int panelW = 230;
            int panelH = 170;
            int panelX = (int)SCREEN_WIDTH - panelW - 15;
            int panelY = 58;
            DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 0, 0, 0, 190 });
            DrawRectangleLines(panelX, panelY, panelW, panelH, (Color){ 100, 110, 130, 255 });

            DrawText(TextFormat("FPS: %d", GetFPS()), panelX + 10, panelY + 10, 14, GREEN);
            DrawText(TextFormat("Pos: %.1f, %.1f", player.position.x, player.position.y), panelX + 10, panelY + 30, 13, RAYWHITE);
            DrawText(TextFormat("Vel: %.1f, %.1f", player.velocity.x, player.velocity.y), panelX + 10, panelY + 50, 13, RAYWHITE);
            DrawText(TextFormat("onGround: %s", player.onGround ? "YES" : "NO"), panelX + 10, panelY + 70, 13, player.onGround ? GREEN : RED);
            DrawText(TextFormat("isJumping: %s", player.isJumping ? "YES" : "NO"), panelX + 10, panelY + 90, 13, player.isJumping ? YELLOW : GRAY);
            DrawText(TextFormat("Coyote: %.2fs", player.coyoteTimer), panelX + 10, panelY + 110, 13, SKYBLUE);
            DrawText(TextFormat("JumpBuf: %.2fs", player.jumpBufferTimer), panelX + 10, panelY + 130, 13, ORANGE);
            DrawText(TextFormat("Sprites: %s", assets.hasPlayerTexture ? "CUSTOM" : "PROCEDURAL"), panelX + 10, panelY + 150, 13, assets.hasPlayerTexture ? GREEN : LIGHTGRAY);
        }

        EndDrawing();
    }

    // Cleanup resources
    AssetsUnload(&assets);
    CloseWindow();
    return 0;
}
