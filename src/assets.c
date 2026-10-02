#include "assets.h"
#include <stdio.h>

void AssetsInit(GameAssets *assets) {
    assets->playerTexture = (Texture2D){ 0 };
    assets->coinTexture = (Texture2D){ 0 };
    assets->flagTexture = (Texture2D){ 0 };
    assets->tilesetTexture = (Texture2D){ 0 };
    assets->hasPlayerTexture = false;
    assets->hasCoinTexture = false;
    assets->hasFlagTexture = false;
    assets->hasTilesetTexture = false;
}

void AssetsLoad(GameAssets *assets) {
    // 1. Player Sprite
    if (FileExists("assets/player.png")) {
        assets->playerTexture = LoadTexture("assets/player.png");
        SetTextureFilter(assets->playerTexture, TEXTURE_FILTER_POINT); // Crisp pixel art
        assets->hasPlayerTexture = (assets->playerTexture.id > 0);
        TraceLog(LOG_INFO, "ASSETS: Loaded assets/player.png (%dx%d)",
                 assets->playerTexture.width, assets->playerTexture.height);
    } else {
        TraceLog(LOG_INFO, "ASSETS: assets/player.png not found, using procedural fallback");
    }

    // 2. Coin Sprite
    if (FileExists("assets/coin.png")) {
        assets->coinTexture = LoadTexture("assets/coin.png");
        SetTextureFilter(assets->coinTexture, TEXTURE_FILTER_POINT);
        assets->hasCoinTexture = (assets->coinTexture.id > 0);
        TraceLog(LOG_INFO, "ASSETS: Loaded assets/coin.png (%dx%d)",
                 assets->coinTexture.width, assets->coinTexture.height);
    } else {
        TraceLog(LOG_INFO, "ASSETS: assets/coin.png not found, using procedural fallback");
    }

    // 3. Flag / Goal Sprite
    if (FileExists("assets/flag.png")) {
        assets->flagTexture = LoadTexture("assets/flag.png");
        SetTextureFilter(assets->flagTexture, TEXTURE_FILTER_POINT);
        assets->hasFlagTexture = (assets->flagTexture.id > 0);
        TraceLog(LOG_INFO, "ASSETS: Loaded assets/flag.png (%dx%d)",
                 assets->flagTexture.width, assets->flagTexture.height);
    } else {
        TraceLog(LOG_INFO, "ASSETS: assets/flag.png not found, using procedural fallback");
    }

    // 4. Platform & Hazard Tileset Sprite
    const char *tilesetPath = NULL;
    if (FileExists("assets/levels/tileset.png")) {
        tilesetPath = "assets/levels/tileset.png";
    } else if (FileExists("assets/tileset.png")) {
        tilesetPath = "assets/tileset.png";
    }

    if (tilesetPath) {
        assets->tilesetTexture = LoadTexture(tilesetPath);
        SetTextureFilter(assets->tilesetTexture, TEXTURE_FILTER_POINT);
        assets->hasTilesetTexture = (assets->tilesetTexture.id > 0);
        TraceLog(LOG_INFO, "ASSETS: Loaded %s (%dx%d)",
                 tilesetPath, assets->tilesetTexture.width, assets->tilesetTexture.height);
    } else {
        TraceLog(LOG_INFO, "ASSETS: tileset.png not found, using procedural fallback");
    }
}

void AssetsUnload(GameAssets *assets) {
    if (assets->hasPlayerTexture) {
        UnloadTexture(assets->playerTexture);
        assets->hasPlayerTexture = false;
    }
    if (assets->hasCoinTexture) {
        UnloadTexture(assets->coinTexture);
        assets->hasCoinTexture = false;
    }
    if (assets->hasFlagTexture) {
        UnloadTexture(assets->flagTexture);
        assets->hasFlagTexture = false;
    }
    if (assets->hasTilesetTexture) {
        UnloadTexture(assets->tilesetTexture);
        assets->hasTilesetTexture = false;
    }
}
