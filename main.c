#include "raylib.h"
#include <stdio.h>

// Scaled up values for frame-time multiplication (pixels per second)
#define GRAVITY 1200.0f
#define JUMP_FORCE 500.0f
#define MOVE_SPEED 300.0f
#define GRAVITY_LIMIT 800.0f

typedef struct {
    float x, y;
    float velocityX, velocityY;
    float width, height;
    bool isJumping;
    bool onGround;
} Player;

Player player;
const float SCREEN_WIDTH = 800.0f;
const float SCREEN_HEIGHT = 400.0f;
const float PLATFORM_HEIGHT = 20.0f;

void initPlayer() {
    player.x = 50.0f;
    player.y = SCREEN_HEIGHT - 100.0f;
    player.width = 40.0f;
    player.height = 60.0f;
    player.velocityX = 0.0f;
    player.velocityY = 0.0f;
    player.isJumping = false;
    player.onGround = true;
}

void updatePlayer(float dt) {
    // Apply gravity
    player.velocityY += GRAVITY * dt;
    
    if (player.velocityY > GRAVITY_LIMIT) {
        player.velocityY = GRAVITY_LIMIT;
    }
    
    // Apply horizontal movement
    if (IsKeyDown(KEY_RIGHT)) {
        player.velocityX = MOVE_SPEED;
    } else if (IsKeyDown(KEY_LEFT)) {
        player.velocityX = -MOVE_SPEED;
    } else {
        player.velocityX = 0.0f;
    }
    
    // Apply jump
    if (IsKeyPressed(KEY_SPACE) && player.onGround) {
        player.velocityY = -JUMP_FORCE;
        player.onGround = false;
    }
    
    // Update position scaled by dt
    player.x += player.velocityX * dt;
    player.y += player.velocityY * dt;
    
    // Check ground collision
    if (player.y >= SCREEN_HEIGHT - PLATFORM_HEIGHT - player.height) {
        player.y = SCREEN_HEIGHT - PLATFORM_HEIGHT - player.height;
        player.velocityY = 0.0f;
        player.onGround = true;
        player.isJumping = false;
    }
    
    // Keep player within screen bounds
    if (player.x < 0) player.x = 0;
    if (player.x > SCREEN_WIDTH - player.width) player.x = SCREEN_WIDTH - player.width;
}

void drawGame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Draw ground
    DrawRectangle(0, SCREEN_HEIGHT - PLATFORM_HEIGHT, SCREEN_WIDTH, PLATFORM_HEIGHT, DARKGRAY);
    
    // Fixed: Draw player as BLUE (RAYWHITE made it invisible against RAYWHITE background!)
    DrawRectangleV((Vector2){player.x, player.y}, (Vector2){player.width, player.height}, BLUE);
    
    // Draw debug text
    DrawText("Press LEFT/RIGHT to move, SPACE to jump", 10, 10, 14, DARKGRAY);
    
    char posText[64];
    sprintf(posText, "Player Pos: X: %.2f | Y: %.2f", player.x, player.y);
    DrawText(posText, 10, 30, 14, DARKGRAY);
    
    EndDrawing();
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "2D Platformer - Jump and Run");
    SetTargetFPS(60);
    
    initPlayer();
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        updatePlayer(dt);
        drawGame();
    }
    
    CloseWindow();
    return 0;
}