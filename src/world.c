#include "world.h"
#include "player.h"
#include "common.h"
#include "assets.h"
#include "raymath.h"
#include <math.h>

void WorldInit(World *world) {
    world->platformCount = 0;
    world->collectibleCount = 0;
    world->totalCoins = 0;

    // Helper macro to add platform
    #define ADD_PLATFORM(px, py, pw, ph, topCol, bodyCol, danger) \
        if (world->platformCount < MAX_PLATFORMS) { \
            world->platforms[world->platformCount].bounds = (Rectangle){ px, py, pw, ph }; \
            world->platforms[world->platformCount].topColor = topCol; \
            world->platforms[world->platformCount].bodyColor = bodyCol; \
            world->platforms[world->platformCount].isDanger = danger; \
            world->platformCount++; \
        }

    // Helper macro to add collectible
    #define ADD_COIN(cx, cy) \
        if (world->collectibleCount < MAX_COLLECTIBLES) { \
            world->collectibles[world->collectibleCount].position = (Vector2){ cx, cy }; \
            world->collectibles[world->collectibleCount].radius = 12.0f; \
            world->collectibles[world->collectibleCount].collected = false; \
            world->collectibles[world->collectibleCount].animOffset = (float)(world->collectibleCount * 0.8f); \
            world->collectibleCount++; \
            world->totalCoins++; \
        }

    Color grassTop   = (Color){ 46, 204, 113, 255 };  // Emerald green
    Color dirtBody   = (Color){ 44, 62, 80, 255 };    // Deep dark slate
    Color stoneTop   = (Color){ 52, 152, 219, 255 };  // Cyan / light slate
    Color stoneBody  = (Color){ 33, 47, 61, 255 };    // Dark midnight stone
    Color dangerTop  = (Color){ 231, 76, 60, 255 };   // Crimson hazard
    Color dangerBody = (Color){ 120, 40, 31, 255 };

    // --- Platforms Layout ---
    // 1. Starting Ground
    ADD_PLATFORM(0, 460, 480, 140, grassTop, dirtBody, false);

    // 2. Stepping stone platforms over First Gap
    ADD_PLATFORM(280, 370, 110, 22, stoneTop, stoneBody, false);
    ADD_PLATFORM(440, 300, 110, 22, stoneTop, stoneBody, false);

    // 3. Second Ground
    ADD_PLATFORM(560, 460, 360, 140, grassTop, dirtBody, false);
    ADD_PLATFORM(640, 360, 120, 22, stoneTop, stoneBody, false);
    ADD_PLATFORM(790, 280, 120, 22, stoneTop, stoneBody, false);

    // 4. Wall-Jump Chimney Shaft & Fortress
    ADD_PLATFORM(980, 420, 420, 180, grassTop, dirtBody, false);
    // Vertical Wall-Jump Shaft (Climb back and forth to reach the high summit!)
    ADD_PLATFORM(1010, 180, 32, 240, stoneTop, stoneBody, false);  // Left wall
    ADD_PLATFORM(1106, 180, 32, 240, stoneTop, stoneBody, false);  // Right wall (64px gap)
    ADD_PLATFORM(990, 150, 160, 24, stoneTop, stoneBody, false);   // Summit platform
    ADD_PLATFORM(1240, 250, 110, 22, stoneTop, stoneBody, false);
    ADD_PLATFORM(1370, 330, 100, 22, stoneTop, stoneBody, false);

    // 5. Hazard Lava Bed in Pit
    ADD_PLATFORM(1450, 570, 430, 30, dangerTop, dangerBody, true);

    // 6. Tall Floating Wall Pillars across the Hazard (Enables wall-sliding & clutch wall-jump recoveries)
    ADD_PLATFORM(1530, 330, 50, 170, stoneTop, stoneBody, false);
    ADD_PLATFORM(1670, 260, 50, 240, stoneTop, stoneBody, false);
    ADD_PLATFORM(1810, 200, 50, 300, stoneTop, stoneBody, false);

    // 7. Victory Plateau
    ADD_PLATFORM(1960, 430, 640, 170, grassTop, dirtBody, false);
    ADD_PLATFORM(2080, 330, 130, 24, stoneTop, stoneBody, false);
    ADD_PLATFORM(2240, 240, 120, 24, stoneTop, stoneBody, false);

    // --- Collectibles (Coins/Gems) ---
    ADD_COIN(330, 330);
    ADD_COIN(490, 260);
    ADD_COIN(700, 320);
    ADD_COIN(850, 240);
    ADD_COIN(1070, 110); // Summit bonus gem above the wall-jump shaft!
    ADD_COIN(1270, 200);
    ADD_COIN(1400, 280);
    ADD_COIN(1555, 280);
    ADD_COIN(1695, 210);
    ADD_COIN(1835, 150);
    ADD_COIN(2140, 290);
    ADD_COIN(2300, 200);

    // --- Goal Banner ---
    world->goal.bounds = (Rectangle){ 2460.0f, 350.0f, 40.0f, 80.0f };
    world->goal.reached = false;
}

void WorldUpdate(World *world, Player *player, float dt) {
    (void)world;
    (void)player; // Handled primarily in player collision sweep
    (void)dt;
}

void WorldDrawBackground(const World *world, Camera2D camera) {
    (void)world;
    // 1. Sky Gradient: Deep space navy to rich twilight blue
    DrawRectangleGradientV(0, 0, (int)WORLD_WIDTH, (int)WORLD_HEIGHT,
        (Color){ 16, 24, 40, 255 },
        (Color){ 30, 45, 75, 255 }
    );

    // 2. Parallax Distant Mountains / Hills (Far layer: parallax factor 0.2)
    float farOffsetX = camera.target.x * 0.2f;
    for (int i = -1; i < 8; i++) {
        float peakX = i * 400.0f - fmodf(farOffsetX, 400.0f);
        Vector2 v1 = { peakX, 500.0f };
        Vector2 v2 = { peakX + 220.0f, 260.0f };
        Vector2 v3 = { peakX + 440.0f, 500.0f };
        DrawTriangle(v1, v3, v2, (Color){ 24, 34, 56, 255 });
    }

    // 3. Parallax Midground Hills (Mid layer: parallax factor 0.45)
    float midOffsetX = camera.target.x * 0.45f;
    for (int i = -1; i < 10; i++) {
        float peakX = i * 320.0f - fmodf(midOffsetX, 320.0f);
        Vector2 v1 = { peakX, 550.0f };
        Vector2 v2 = { peakX + 180.0f, 340.0f };
        Vector2 v3 = { peakX + 360.0f, 550.0f };
        DrawTriangle(v1, v3, v2, (Color){ 35, 48, 76, 255 });
    }

    // 4. Subtle Stars / Ambient Lights in sky
    for (int i = 0; i < 40; i++) {
        float starX = (float)((i * 137) % (int)WORLD_WIDTH);
        float starY = (float)((i * 83) % 240);
        DrawCircle((int)starX, (int)starY, 1.2f, (Color){ 255, 255, 255, 120 });
    }
}

void WorldDrawForeground(const World *world, const GameAssets *assets) {
    // 1. Draw Platforms
    for (int i = 0; i < world->platformCount; i++) {
        Rectangle r = world->platforms[i].bounds;
        Color bodyColor = world->platforms[i].bodyColor;
        Color topColor  = world->platforms[i].topColor;

        // Platform Main Body
        DrawRectangleRec(r, bodyColor);

        // Platform Top Highlight Strip (Grass/Edge)
        float topThickness = (r.height < 30.0f) ? 5.0f : 8.0f;
        DrawRectangle((int)r.x, (int)r.y, (int)r.width, (int)topThickness, topColor);

        // Platform subtle bevel line
        DrawLine((int)r.x, (int)(r.y + topThickness), (int)(r.x + r.width), (int)(r.y + topThickness), (Color){ 0, 0, 0, 50 });

        // Hazard indicator glow
        if (world->platforms[i].isDanger) {
            float pulse = (sinf((float)GetTime() * 6.0f) + 1.0f) * 0.5f;
            DrawRectangleLinesEx(r, 2.0f, (Color){ 255, 100, 100, (unsigned char)(120 + 80 * pulse) });
        }
    }

    // 2. Draw Collectibles (Coins/Gems)
    float time = (float)GetTime();
    for (int i = 0; i < world->collectibleCount; i++) {
        const Collectible *c = &world->collectibles[i];
        if (c->collected) continue;

        float bobY = sinf(time * 3.5f + c->animOffset) * 4.0f;
        Vector2 pos = { c->position.x, c->position.y + bobY };

        // Outer glow
        DrawCircleV(pos, c->radius + 4.0f, (Color){ 241, 196, 15, 60 });

        if (assets && assets->hasCoinTexture) {
            Rectangle sourceRec = { 0.0f, 0.0f, (float)assets->coinTexture.width, (float)assets->coinTexture.height };
            float size = c->radius * 2.0f;
            Rectangle destRec = { pos.x - c->radius, pos.y - c->radius, size, size };
            DrawTexturePro(assets->coinTexture, sourceRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
        } else {
            // Procedural coin body (diamond / circle)
            DrawCircleV(pos, c->radius, (Color){ 241, 196, 15, 255 });
            // Coin inner highlight
            DrawCircleV((Vector2){ pos.x - 2.0f, pos.y - 2.0f }, c->radius * 0.45f, (Color){ 255, 243, 176, 255 });
        }
    }

    // 3. Draw Goal Flag
    Rectangle g = world->goal.bounds;
    if (assets && assets->hasFlagTexture) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)assets->flagTexture.width, (float)assets->flagTexture.height };
        DrawTexturePro(assets->flagTexture, sourceRec, g, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    } else {
        // Procedural Flag: Pole
        DrawRectangle((int)g.x, (int)g.y, 6, (int)g.height, (Color){ 189, 195, 199, 255 });
        // Flag banner
        Color flagColor = world->goal.reached ? (Color){ 46, 204, 113, 255 } : (Color){ 231, 76, 60, 255 };
        float waveOffset = sinf(time * 4.0f) * 3.0f;
        Vector2 p1 = { g.x + 6, g.y };
        Vector2 p2 = { g.x + 6, g.y + 34.0f };
        Vector2 p3 = { g.x + 36.0f + waveOffset, g.y + 17.0f };
        DrawTriangle(p1, p2, p3, flagColor);

        // Goal base
        DrawRectangle((int)(g.x - 6), (int)(g.y + g.height - 4), 18, 6, (Color){ 127, 140, 141, 255 });
    }
}
