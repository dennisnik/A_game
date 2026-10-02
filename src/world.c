#include "world.h"
#include "player.h"
#include "common.h"
#include "assets.h"
#include "json.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Plain solid fallback colors for platforms when no tileset texture is loaded
static const Color FALLBACK_GRASS  = (Color){ 34, 197, 94, 255 };   // Plain green
static const Color FALLBACK_DANGER = (Color){ 239, 68, 68, 255 };   // Plain red
static const Color FALLBACK_STONE  = (Color){ 100, 116, 139, 255 };  // Plain stone slate grey

void WorldInitDefault(World *world) {
    if (world->tileGrid) {
        free(world->tileGrid);
        world->tileGrid = NULL;
    }
    world->mapWidth = 0;
    world->mapHeight = 0;
    world->tileWidth = 32;
    world->tileHeight = 32;
    world->platformCount = 0;
    world->collectibleCount = 0;
    world->totalCoins = 0;
    world->worldWidth = WORLD_WIDTH;
    world->worldHeight = WORLD_HEIGHT;
    world->loadedFromMap = false;

    // Helper macro to add platform
    #define ADD_PLATFORM(px, py, pw, ph, col, danger) \
        if (world->platformCount < MAX_PLATFORMS) { \
            world->platforms[world->platformCount].bounds = (Rectangle){ px, py, pw, ph }; \
            world->platforms[world->platformCount].color = col; \
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

    // --- Platforms Layout ---
    // 1. Starting Ground
    ADD_PLATFORM(0, 460, 480, 140, FALLBACK_GRASS, false);

    // 2. Stepping stone platforms over First Gap
    ADD_PLATFORM(280, 370, 110, 22, FALLBACK_STONE, false);
    ADD_PLATFORM(440, 300, 110, 22, FALLBACK_STONE, false);

    // 3. Second Ground
    ADD_PLATFORM(560, 460, 360, 140, FALLBACK_GRASS, false);
    ADD_PLATFORM(640, 360, 120, 22, FALLBACK_STONE, false);
    ADD_PLATFORM(790, 280, 120, 22, FALLBACK_STONE, false);

    // 4. Wall-Jump Chimney Shaft & Fortress
    ADD_PLATFORM(980, 420, 420, 180, FALLBACK_GRASS, false);
    // Vertical Wall-Jump Shaft (Climb back and forth to reach the high summit!)
    ADD_PLATFORM(1010, 180, 32, 240, FALLBACK_STONE, false);  // Left wall
    ADD_PLATFORM(1106, 180, 32, 240, FALLBACK_STONE, false);  // Right wall (64px gap)
    ADD_PLATFORM(990, 150, 160, 24, FALLBACK_STONE, false);   // Summit platform
    ADD_PLATFORM(1240, 250, 110, 22, FALLBACK_STONE, false);
    ADD_PLATFORM(1370, 330, 100, 22, FALLBACK_STONE, false);

    // 5. Hazard Lava Bed in Pit
    ADD_PLATFORM(1450, 570, 430, 30, FALLBACK_DANGER, true);

    // 6. Tall Floating Wall Pillars across the Hazard (Enables wall-sliding & clutch wall-jump recoveries)
    ADD_PLATFORM(1530, 330, 50, 170, FALLBACK_STONE, false);
    ADD_PLATFORM(1670, 260, 50, 240, FALLBACK_STONE, false);
    ADD_PLATFORM(1810, 200, 50, 300, FALLBACK_STONE, false);

    // 7. Victory Plateau
    ADD_PLATFORM(1960, 430, 640, 170, FALLBACK_GRASS, false);
    ADD_PLATFORM(2080, 330, 130, 24, FALLBACK_STONE, false);
    ADD_PLATFORM(2240, 240, 120, 24, FALLBACK_STONE, false);

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

static bool StringContainsCase(const char *haystack, const char *needle) {
    if (!haystack || !needle) return false;
    size_t nlen = strlen(needle);
    size_t hlen = strlen(haystack);
    if (nlen > hlen) return false;
    for (size_t i = 0; i <= hlen - nlen; i++) {
        if (strncasecmp(haystack + i, needle, nlen) == 0) return true;
    }
    return false;
}

static int IdentifyEntityType(const char *name, int gid, const JsonValue *root) {
    // 1. Check object name
    if (name && *name) {
        if (StringContainsCase(name, "player")) return 1; // Player
        if (StringContainsCase(name, "coin") || StringContainsCase(name, "gem")) return 2; // Coin
        if (StringContainsCase(name, "goal") || StringContainsCase(name, "flag")) return 3; // Goal
    }

    // 2. Check GID by looking up the image associated with the tile in tilesets
    if (gid > 0 && root) {
        JsonValue *tilesets = JsonObjectGetArray(root, "tilesets");
        if (tilesets) {
            JsonElement *tsElem = tilesets->arrHead;
            while (tsElem) {
                int firstgid = (int)JsonObjectGetNumber(tsElem->value, "firstgid", 0);
                JsonValue *tilesArr = JsonObjectGetArray(tsElem->value, "tiles");
                if (tilesArr && gid >= firstgid) {
                    JsonElement *tElem = tilesArr->arrHead;
                    while (tElem) {
                        int tid = (int)JsonObjectGetNumber(tElem->value, "id", -1);
                        if (firstgid + tid == gid) {
                            const char *img = JsonObjectGetString(tElem->value, "image", "");
                            if (StringContainsCase(img, "player")) return 1;
                            if (StringContainsCase(img, "coin")) return 2;
                            if (StringContainsCase(img, "flag")) return 3;
                        }
                        tElem = tElem->next;
                    }
                }
                tsElem = tsElem->next;
            }
        }
    }
    return 0; // Unknown
}

bool WorldLoadFromTiledJSON(World *world, Player *player, const char *filepath) {
    if (!FileExists(filepath)) return false;

    char *fileText = LoadFileText(filepath);
    if (!fileText) return false;

    JsonValue *root = JsonParse(fileText);
    UnloadFileText(fileText);
    if (!root || root->type != JSON_OBJECT) {
        if (root) JsonFree(root);
        return false;
    }

    int mapW = (int)JsonObjectGetNumber(root, "width", 80);
    int mapH = (int)JsonObjectGetNumber(root, "height", 20);
    int tileW = (int)JsonObjectGetNumber(root, "tilewidth", 32);
    int tileH = (int)JsonObjectGetNumber(root, "tileheight", 32);

    if (world->tileGrid) {
        free(world->tileGrid);
        world->tileGrid = NULL;
    }
    world->mapWidth = mapW;
    world->mapHeight = mapH;
    world->tileWidth = tileW;
    world->tileHeight = tileH;
    world->tileGrid = (int *)calloc(mapW * mapH, sizeof(int));

    world->worldWidth = (float)(mapW * tileW);
    world->worldHeight = (float)(mapH * tileH);
    world->platformCount = 0;
    world->collectibleCount = 0;
    world->totalCoins = 0;

    JsonValue *layersArr = JsonObjectGetArray(root, "layers");
    if (layersArr) {
        JsonElement *layerElem = layersArr->arrHead;
        while (layerElem) {
            JsonValue *layer = layerElem->value;
            const char *layerType = JsonObjectGetString(layer, "type", "");

            if (strcmp(layerType, "tilelayer") == 0) {
                JsonValue *dataArr = JsonObjectGetArray(layer, "data");
                if (dataArr && world->tileGrid) {
                    int idx = 0;
                    JsonElement *dElem = dataArr->arrHead;
                    while (dElem && idx < mapW * mapH) {
                        if (dElem->value->type == JSON_NUMBER) {
                            world->tileGrid[idx] = (int)dElem->value->numVal;
                        }
                        idx++;
                        dElem = dElem->next;
                    }

                    // Coalesce horizontal tiles of the same type into single rectangles for collision
                    for (int y = 0; y < mapH; y++) {
                        for (int x = 0; x < mapW; x++) {
                            int tileVal = world->tileGrid[y * mapW + x];
                            if (tileVal > 0) {
                                int startX = x;
                                while (x + 1 < mapW && world->tileGrid[y * mapW + (x + 1)] == tileVal) {
                                    x++;
                                }
                                int span = x - startX + 1;
                                if (world->platformCount < MAX_PLATFORMS) {
                                    Platform *p = &world->platforms[world->platformCount++];
                                    p->bounds = (Rectangle){
                                        (float)(startX * tileW),
                                        (float)(y * tileH),
                                        (float)(span * tileW),
                                        (float)tileH
                                    };
                                    p->isDanger = (tileVal == 2);
                                    if (p->isDanger) {
                                        p->color = FALLBACK_DANGER;
                                    } else if (tileVal == 3) {
                                        p->color = FALLBACK_STONE;
                                    } else {
                                        p->color = FALLBACK_GRASS;
                                    }
                                }
                            }
                        }
                    }
                }
            } else if (strcmp(layerType, "objectgroup") == 0) {
                JsonValue *objsArr = JsonObjectGetArray(layer, "objects");
                if (objsArr) {
                    JsonElement *objElem = objsArr->arrHead;
                    while (objElem) {
                        JsonValue *obj = objElem->value;
                        const char *name = JsonObjectGetString(obj, "name", "");
                        float ox = (float)JsonObjectGetNumber(obj, "x", 0);
                        float oy = (float)JsonObjectGetNumber(obj, "y", 0);
                        float ow = (float)JsonObjectGetNumber(obj, "width", 32);
                        float oh = (float)JsonObjectGetNumber(obj, "height", 32);
                        int gid = (int)JsonObjectGetNumber(obj, "gid", 0);

                        // In Tiled, objects with a tile 'gid' are anchored at the bottom-left
                        if (gid > 0) {
                            oy -= oh;
                        }

                        int entityType = IdentifyEntityType(name, gid, root);
                        if (entityType == 1) { // Player Start
                            if (player) {
                                player->position = (Vector2){ ox, oy };
                                player->respawnPos = (Vector2){ ox, oy };
                            }
                        } else if (entityType == 2) { // Coin
                            if (world->collectibleCount < MAX_COLLECTIBLES) {
                                Collectible *c = &world->collectibles[world->collectibleCount++];
                                c->position = (Vector2){ ox + ow * 0.5f, oy + oh * 0.5f };
                                c->radius = 12.0f;
                                c->collected = false;
                                c->animOffset = (float)(world->collectibleCount * 0.8f);
                                world->totalCoins++;
                            }
                        } else if (entityType == 3) { // Goal Flag
                            world->goal.bounds = (Rectangle){ ox, oy, ow > 0 ? ow : 40.0f, oh > 0 ? oh : 80.0f };
                            world->goal.reached = false;
                        }

                        objElem = objElem->next;
                    }
                }
            }

            layerElem = layerElem->next;
        }
    }

    JsonFree(root);
    world->loadedFromMap = true;
    TraceLog(LOG_INFO, "TILED: Loaded map '%s' (%dx%d tiles, %d platforms, %d coins)",
             filepath, mapW, mapH, world->platformCount, world->totalCoins);
    return true;
}

void WorldInit(World *world) {
    if (!world) return;
    memset(world, 0, sizeof(World));
    WorldInitDefault(world);
}

void WorldUnload(World *world) {
    if (world->tileGrid) {
        free(world->tileGrid);
        world->tileGrid = NULL;
    }
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
    // 1. Draw Platforms / Level Tiles
    if (assets && assets->hasTilesetTexture && world->tileGrid) {
        int tileW = world->tileWidth > 0 ? world->tileWidth : 32;
        int tileH = world->tileHeight > 0 ? world->tileHeight : 32;
        int cols = assets->tilesetTexture.width / tileW;
        if (cols <= 0) cols = 1;

        // Solid underlay behind platform bounds prevents background mountains/sky from flashing through tile seams
        for (int i = 0; i < world->platformCount; i++) {
            DrawRectangleRec(world->platforms[i].bounds, world->platforms[i].color);
        }

        for (int y = 0; y < world->mapHeight; y++) {
            for (int x = 0; x < world->mapWidth; x++) {
                int tileVal = world->tileGrid[y * world->mapWidth + x];
                if (tileVal <= 0) continue;

                int tileIdx = tileVal - 1; // 0-indexed tile id in tileset
                int col = tileIdx % cols;
                int row = tileIdx / cols;

                Rectangle srcRec = {
                    (float)(col * tileW),
                    (float)(row * tileH),
                    (float)tileW,
                    (float)tileH
                };
                Rectangle destRec = {
                    (float)(x * tileW),
                    (float)(y * tileH),
                    (float)tileW,
                    (float)tileH
                };

                DrawTexturePro(assets->tilesetTexture, srcRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
            }
        }
    } else {
        // Procedural Fallback: Plain solid one-colored platform rectangles
        for (int i = 0; i < world->platformCount; i++) {
            Rectangle r = world->platforms[i].bounds;
            DrawRectangleRec(r, world->platforms[i].color);
        }
    }

    // 2. Draw Collectibles (Coins/Gems)
    float time = (float)GetTime();
    for (int i = 0; i < world->collectibleCount; i++) {
        const Collectible *c = &world->collectibles[i];
        if (c->collected) continue;

        float bobY = sinf(time * 3.5f + c->animOffset) * 4.0f;
        Vector2 pos = { c->position.x, c->position.y + bobY };

        if (assets && assets->hasCoinTexture) {
            // Outer subtle glow
            DrawCircleV(pos, c->radius + 4.0f, (Color){ 241, 196, 15, 60 });
            Rectangle sourceRec = { 0.0f, 0.0f, (float)assets->coinTexture.width, (float)assets->coinTexture.height };
            float size = c->radius * 2.0f;
            Rectangle destRec = { pos.x - c->radius, pos.y - c->radius, size, size };
            DrawTexturePro(assets->coinTexture, sourceRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
        } else {
            // Fallback: Plain yellow circle
            DrawCircleV(pos, c->radius, (Color){ 250, 204, 21, 255 });
        }
    }

    // 3. Draw Goal Flag
    Rectangle g = world->goal.bounds;
    if (assets && assets->hasFlagTexture) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)assets->flagTexture.width, (float)assets->flagTexture.height };
        DrawTexturePro(assets->flagTexture, sourceRec, g, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    } else {
        // Fallback: Plain solid colored banner (red normally, green when reached)
        Color flagColor = world->goal.reached ? (Color){ 34, 197, 94, 255 } : (Color){ 239, 68, 68, 255 };
        DrawRectangleRec(g, flagColor);
    }
}
