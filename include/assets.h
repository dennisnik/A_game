#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"
#include <stdbool.h>

typedef struct GameAssets {
    Texture2D playerTexture;
    Texture2D coinTexture;
    Texture2D flagTexture;
    Texture2D tilesetTexture;
    bool hasPlayerTexture;
    bool hasCoinTexture;
    bool hasFlagTexture;
    bool hasTilesetTexture;
} GameAssets;

void AssetsInit(GameAssets *assets);
void AssetsLoad(GameAssets *assets);
void AssetsUnload(GameAssets *assets);

#endif // ASSETS_H
