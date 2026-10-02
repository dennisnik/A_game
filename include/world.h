#ifndef WORLD_H
#define WORLD_H

#include "raylib.h"
#include <stdbool.h>

typedef struct Player Player;
typedef struct GameAssets GameAssets;

#define MAX_PLATFORMS 128
#define MAX_COLLECTIBLES 64

typedef struct {
    Rectangle bounds;
    Color color;
    bool isDanger; // If true, touching causes respawn
} Platform;

typedef struct {
    Vector2 position;
    float radius;
    bool collected;
    float animOffset;
} Collectible;

typedef struct {
    Rectangle bounds;
    bool reached;
} Goal;

typedef struct World {
    Platform platforms[MAX_PLATFORMS];
    int platformCount;
    Collectible collectibles[MAX_COLLECTIBLES];
    int collectibleCount;
    Goal goal;
    int totalCoins;
    float worldWidth;
    float worldHeight;
    bool loadedFromMap;
    int mapWidth;
    int mapHeight;
    int tileWidth;
    int tileHeight;
    int *tileGrid;
} World;

void WorldInit(World *world);
void WorldUnload(World *world);
bool WorldLoadFromTiledJSON(World *world, Player *player, const char *filepath);
void WorldUpdate(World *world, Player *player, float dt);
void WorldDrawBackground(const World *world, Camera2D camera);
void WorldDrawForeground(const World *world, const GameAssets *assets);

#endif // WORLD_H
