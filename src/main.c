#include "raylib.h"
#include "common.h"
#include "player.h"
#include "world.h"
#include "particles.h"
#include "assets.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow((int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, "Name of the Game");
    SetWindowMinSize(400, 225);
    SetTargetFPS(60);

    // Virtual render target for integer/aspect-ratio scaling in fullscreen
    RenderTexture2D target = LoadRenderTexture((int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

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
        if (IsKeyPressed(KEY_F11) || IsKeyPressed(KEY_F) ||
            ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER))) {
            ToggleFullscreen();
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

        // 1. Render gameplay and UI onto the virtual canvas (800x450)
        BeginTextureMode(target);
            ClearBackground((Color){ 16, 24, 40, 255 });

            // World (Camera space)
            BeginMode2D(camera);
                WorldDrawBackground(&world, camera);
                WorldDrawForeground(&world, &assets);
                ParticleSystemDraw();
                PlayerDraw(&player, &assets);
            EndMode2D();

            // UI / HUD (Screen space inside virtual canvas)
            // Top banner background
            DrawRectangle(0, 0, (int)SCREEN_WIDTH, 48, (Color){ 10, 15, 26, 180 });
            DrawLine(0, 48, (int)SCREEN_WIDTH, 48, (Color){ 52, 73, 94, 120 });

            // HUD Text
            DrawText(TextFormat("SCORE: %04d", player.score), 20, 15, 20, RAYWHITE);
            DrawCircle(195, 25, 7.0f, (Color){ 241, 196, 15, 255 });
            DrawText(TextFormat("%d / %d GEMS", player.score / 100, world.totalCoins), 210, 16, 17, (Color){ 241, 196, 15, 255 });
            DrawText(TextFormat("DEATHS: %d", player.deathCount), 365, 16, 17, (Color){ 231, 76, 60, 255 });

            // Bottom control guide
            DrawText("[A/D or Arrows] Move   |   [Space/W] Jump / Wall-Jump   |   [F/F11] Fullscreen   |   [R] Restart   |   [F3] Debug",
                     20, (int)SCREEN_HEIGHT - 25, 13, (Color){ 189, 195, 199, 210 });

            // Goal Reached Banner
            if (player.reachedGoal) {
                DrawRectangle(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, (Color){ 0, 0, 0, 140 });
                DrawRectangleRounded((Rectangle){ SCREEN_WIDTH * 0.5f - 220, SCREEN_HEIGHT * 0.5f - 90, 440, 180 }, 0.2f, 6, (Color){ 24, 34, 56, 240 });
                DrawRectangleRoundedLines((Rectangle){ SCREEN_WIDTH * 0.5f - 220, SCREEN_HEIGHT * 0.5f - 90, 440, 180 }, 0.2f, 6, (Color){ 46, 204, 113, 255 });

                DrawText("STAGE CLEAR!", (int)(SCREEN_WIDTH * 0.5f - 120), (int)(SCREEN_HEIGHT * 0.5f - 60), 34, (Color){ 46, 204, 113, 255 });
                DrawText(TextFormat("Final Score: %d  |  Deaths: %d", player.score, player.deathCount), (int)(SCREEN_WIDTH * 0.5f - 120), (int)(SCREEN_HEIGHT * 0.5f - 15), 18, RAYWHITE);
                DrawText("Press [R] to Play Again", (int)(SCREEN_WIDTH * 0.5f - 95), (int)(SCREEN_HEIGHT * 0.5f + 30), 16, (Color){ 241, 196, 15, 255 });
            }

            // Debug info HUD
            if (showDebug) {
                int panelW = 230;
                int panelH = 205;
                int panelX = (int)SCREEN_WIDTH - panelW - 15;
                int panelY = 58;
                DrawRectangle(panelX, panelY, panelW, panelH, (Color){ 0, 0, 0, 190 });
                DrawRectangleLines(panelX, panelY, panelW, panelH, (Color){ 100, 110, 130, 255 });

                DrawText(TextFormat("FPS: %d", GetFPS()), panelX + 10, panelY + 10, 14, GREEN);
                DrawText(TextFormat("Pos: %.1f, %.1f", player.position.x, player.position.y), panelX + 10, panelY + 28, 13, RAYWHITE);
                DrawText(TextFormat("Vel: %.1f, %.1f", player.velocity.x, player.velocity.y), panelX + 10, panelY + 46, 13, RAYWHITE);
                DrawText(TextFormat("onGround: %s", player.onGround ? "YES" : "NO"), panelX + 10, panelY + 64, 13, player.onGround ? GREEN : RED);
                DrawText(TextFormat("WallSlide: %s", player.isWallSliding ? "YES" : "NO"), panelX + 10, panelY + 82, 13, player.isWallSliding ? YELLOW : GRAY);
                DrawText(TextFormat("WallDir: %d", player.wallDirection), panelX + 10, panelY + 100, 13, SKYBLUE);
                DrawText(TextFormat("WallCoyote: %.2fs", player.wallCoyoteTimer), panelX + 10, panelY + 118, 13, SKYBLUE);
                DrawText(TextFormat("JumpBuf: %.2fs", player.jumpBufferTimer), panelX + 10, panelY + 136, 13, ORANGE);
                DrawText(TextFormat("Deaths: %d", player.deathCount), panelX + 10, panelY + 154, 13, (Color){ 231, 76, 60, 255 });
                DrawText(TextFormat("Sprites: %s", assets.hasPlayerTexture ? "CUSTOM" : "PROCEDURAL"), panelX + 10, panelY + 172, 13, assets.hasPlayerTexture ? GREEN : LIGHTGRAY);
            }
        EndTextureMode();

        // 2. Render Virtual Canvas to Screen with aspect-ratio letterboxing
        BeginDrawing();
            ClearBackground(BLACK);

            float screenW = (float)GetScreenWidth();
            float screenH = (float)GetScreenHeight();
            float scale = fminf(screenW / SCREEN_WIDTH, screenH / SCREEN_HEIGHT);

            // Note: Render textures in OpenGL are Y-flipped, so negative height corrects orientation
            Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
            Rectangle destRec = {
                (screenW - (SCREEN_WIDTH * scale)) * 0.5f,
                (screenH - (SCREEN_HEIGHT * scale)) * 0.5f,
                SCREEN_WIDTH * scale,
                SCREEN_HEIGHT * scale
            };

            DrawTexturePro(target.texture, sourceRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
        EndDrawing();
    }

    // Cleanup resources
    UnloadRenderTexture(target);
    AssetsUnload(&assets);
    CloseWindow();
    return 0;
}
