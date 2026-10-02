#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include <stdbool.h>

typedef struct World World;
typedef struct GameAssets GameAssets;

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    Vector2 respawnPos;
    
    bool onGround;
    bool isJumping;
    float coyoteTimer;
    float jumpBufferTimer;
    
    // Wall Slide & Wall Jump state
    bool isWallSliding;
    int wallDirection;        // -1: wall on left, +1: wall on right, 0: none
    int lastWallDirection;    // Cached for wall coyote time
    float wallCoyoteTimer;    // Window to wall jump after leaving wall
    float wallJumpLockTimer;  // Input lock right after wall jumping

    int facing;          // -1 (left) or 1 (right)
    Vector2 stretch;     // (x, y) scale factors for juice squash/stretch
    
    int score;
    int deathCount;
    bool reachedGoal;
} Player;

void PlayerInit(Player *player, Vector2 startPos);
void PlayerUpdate(Player *player, World *world, float dt);
void PlayerDraw(const Player *player, const GameAssets *assets);
void PlayerRespawn(Player *player);

#endif // PLAYER_H
