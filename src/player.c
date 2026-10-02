#include "player.h"
#include "world.h"
#include "particles.h"
#include "common.h"
#include "assets.h"
#include "raymath.h"
#include <math.h>

static bool CheckAABB(Rectangle a, Rectangle b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void PlayerInit(Player *player, Vector2 startPos) {
    player->position = startPos;
    player->respawnPos = startPos;
    player->velocity = (Vector2){ 0.0f, 0.0f };
    player->size = (Vector2){ 32.0f, 48.0f };
    player->onGround = false;
    player->isJumping = false;
    player->coyoteTimer = 0.0f;
    player->jumpBufferTimer = 0.0f;

    player->isWallSliding = false;
    player->wallDirection = 0;
    player->lastWallDirection = 0;
    player->wallCoyoteTimer = 0.0f;
    player->wallJumpLockTimer = 0.0f;

    player->facing = 1;
    player->stretch = (Vector2){ 1.0f, 1.0f };
    player->score = 0;
    player->deathCount = 0;
    player->reachedGoal = false;
}

void PlayerRespawn(Player *player) {
    player->position = player->respawnPos;
    player->velocity = (Vector2){ 0.0f, 0.0f };
    player->onGround = false;
    player->isJumping = false;
    player->coyoteTimer = 0.0f;
    player->jumpBufferTimer = 0.0f;

    player->isWallSliding = false;
    player->wallDirection = 0;
    player->lastWallDirection = 0;
    player->wallCoyoteTimer = 0.0f;
    player->wallJumpLockTimer = 0.0f;

    player->stretch = (Vector2){ 1.0f, 1.0f };
    player->deathCount++;
    ParticleSpawnBurst(player->position, 16, (Color){ 64, 150, 255, 255 }, 50.0f, 120.0f, 0.4f);
}

void PlayerUpdate(Player *player, World *world, float dt) {
    // 1. Horizontal Input & Wall Jump Lockout
    int moveInput = 0;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInput += 1;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  moveInput -= 1;

    if (player->wallJumpLockTimer > 0.0f) {
        player->wallJumpLockTimer -= dt;
        // During lock timer, we don't allow counter-steering back toward the wall
    } else {
        if (moveInput != 0) {
            player->facing = moveInput;
        }

        float targetSpeed = moveInput * MOVE_SPEED;
        float accel = player->onGround ? GROUND_ACCEL : AIR_ACCEL;
        float friction = player->onGround ? GROUND_FRICTION : AIR_DRAG;

        if (moveInput != 0) {
            if (player->velocity.x < targetSpeed) {
                player->velocity.x += accel * dt;
                if (player->velocity.x > targetSpeed) player->velocity.x = targetSpeed;
            } else if (player->velocity.x > targetSpeed) {
                player->velocity.x -= accel * dt;
                if (player->velocity.x < targetSpeed) player->velocity.x = targetSpeed;
            }
        } else {
            if (player->velocity.x > 0.0f) {
                player->velocity.x -= friction * dt;
                if (player->velocity.x < 0.0f) player->velocity.x = 0.0f;
            } else if (player->velocity.x < 0.0f) {
                player->velocity.x += friction * dt;
                if (player->velocity.x > 0.0f) player->velocity.x = 0.0f;
            }
        }
    }

    // Occasional running dust when moving fast on ground
    if (player->onGround && fabsf(player->velocity.x) > 150.0f) {
        static float runDustTimer = 0.0f;
        runDustTimer += dt;
        if (runDustTimer > 0.12f) {
            runDustTimer = 0.0f;
            Vector2 dustPos = {
                player->position.x + (player->facing == 1 ? 4.0f : player->size.x - 4.0f),
                player->position.y + player->size.y - 2.0f
            };
            Vector2 dustVel = { -player->facing * 30.0f, -20.0f };
            ParticleSpawn(dustPos, dustVel, (Color){ 200, 200, 200, 180 }, 3.5f, 0.25f);
        }
    }

    // 2. Wall Detection Probes (Left & Right)
    int touchingWall = 0;
    if (!player->onGround) {
        Rectangle leftProbe = { player->position.x - 3.0f, player->position.y + 6.0f, 3.0f, player->size.y - 12.0f };
        Rectangle rightProbe = { player->position.x + player->size.x, player->position.y + 6.0f, 3.0f, player->size.y - 12.0f };

        for (int i = 0; i < world->platformCount; i++) {
            if (world->platforms[i].isDanger) continue;
            Rectangle plat = world->platforms[i].bounds;
            if (CheckAABB(leftProbe, plat)) {
                touchingWall = -1;
                break;
            }
            if (CheckAABB(rightProbe, plat)) {
                touchingWall = 1;
                break;
            }
        }
    }

    // Wall slide state & coyote timer
    if (touchingWall != 0 && !player->onGround) {
        player->wallDirection = touchingWall;
        player->lastWallDirection = touchingWall;
        player->wallCoyoteTimer = WALL_COYOTE_TIME;

        // Wall slide is active if falling
        if (player->velocity.y > 0.0f) {
            player->isWallSliding = true;
            if (player->velocity.y > WALL_SLIDE_SPEED) {
                player->velocity.y = WALL_SLIDE_SPEED;
            }

            // Wall slide friction dust
            static float wallDustTimer = 0.0f;
            wallDustTimer += dt;
            if (wallDustTimer > 0.09f) {
                wallDustTimer = 0.0f;
                Vector2 wallDustPos = {
                    (touchingWall == 1) ? player->position.x + player->size.x : player->position.x,
                    player->position.y + player->size.y * 0.7f
                };
                Vector2 wallDustVel = { -touchingWall * 20.0f, -15.0f };
                ParticleSpawn(wallDustPos, wallDustVel, (Color){ 220, 220, 220, 160 }, 3.0f, 0.22f);
            }
        } else {
            player->isWallSliding = false;
        }
    } else {
        player->isWallSliding = false;
        player->wallDirection = 0;
        if (player->wallCoyoteTimer > 0.0f) {
            player->wallCoyoteTimer -= dt;
            if (player->wallCoyoteTimer < 0.0f) player->wallCoyoteTimer = 0.0f;
        }
    }

    // 3. Vertical Physics (Gravity & Limits)
    if (!player->isWallSliding) {
        player->velocity.y += GRAVITY * dt;
        if (player->velocity.y > GRAVITY_LIMIT) {
            player->velocity.y = GRAVITY_LIMIT;
        }
    }

    // 4. Coyote Time & Jump Buffering
    if (player->onGround) {
        player->coyoteTimer = COYOTE_TIME;
    } else {
        player->coyoteTimer -= dt;
        if (player->coyoteTimer < 0.0f) player->coyoteTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        player->jumpBufferTimer = JUMP_BUFFER_TIME;
    }
    if (player->jumpBufferTimer > 0.0f) {
        player->jumpBufferTimer -= dt;
        if (player->jumpBufferTimer < 0.0f) player->jumpBufferTimer = 0.0f;
    }

    // 5. Jump Execution: Standard Ground Jump OR Wall Jump!
    if (player->jumpBufferTimer > 0.0f) {
        if (player->onGround || player->coyoteTimer > 0.0f) {
            // Normal Ground Jump
            player->velocity.y = -JUMP_FORCE;
            player->onGround = false;
            player->isJumping = true;
            player->coyoteTimer = 0.0f;
            player->jumpBufferTimer = 0.0f;
            player->stretch = (Vector2){ 0.75f, 1.30f }; // Vertical stretch

            Vector2 feetPos = { player->position.x + player->size.x * 0.5f, player->position.y + player->size.y };
            ParticleSpawnBurst(feetPos, 7, (Color){ 220, 220, 220, 220 }, 25.0f, 85.0f, 0.28f);
        } else if (player->wallDirection != 0 || player->wallCoyoteTimer > 0.0f) {
            // Wall Jump!
            int jumpWall = (player->wallDirection != 0) ? player->wallDirection : player->lastWallDirection;

            player->velocity.x = -jumpWall * WALL_JUMP_FORCE_X;
            player->velocity.y = -WALL_JUMP_FORCE_Y;
            player->onGround = false;
            player->isJumping = true;
            player->isWallSliding = false;
            player->wallDirection = 0;
            player->wallCoyoteTimer = 0.0f;
            player->jumpBufferTimer = 0.0f;
            player->coyoteTimer = 0.0f;
            player->facing = -jumpWall; // Face away from wall
            player->wallJumpLockTimer = WALL_JUMP_LOCK_TIME;
            player->stretch = (Vector2){ 0.70f, 1.35f };

            // Wall kick dust burst
            Vector2 kickPos = {
                (jumpWall == 1) ? player->position.x + player->size.x : player->position.x,
                player->position.y + player->size.y * 0.5f
            };
            ParticleSpawnBurst(kickPos, 9, (Color){ 220, 220, 220, 220 }, 35.0f, 110.0f, 0.26f);
        }
    }

    // 6. Variable Jump Height: Cut upward velocity early on key release
    if ((IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_UP) || IsKeyReleased(KEY_W)) && player->velocity.y < -50.0f) {
        player->velocity.y *= JUMP_RELEASE_DAMPING;
    }

    // 5. Separate Axis Collision Resolution (X-axis first, then Y-axis)
    // --- X Axis Movement ---
    player->position.x += player->velocity.x * dt;
    Rectangle playerRectX = { player->position.x, player->position.y, player->size.x, player->size.y };

    for (int i = 0; i < world->platformCount; i++) {
        Rectangle plat = world->platforms[i].bounds;
        if (CheckAABB(playerRectX, plat)) {
            if (world->platforms[i].isDanger) {
                PlayerRespawn(player);
                return;
            }
            if (player->velocity.x > 0.0f) {
                player->position.x = plat.x - player->size.x;
            } else if (player->velocity.x < 0.0f) {
                player->position.x = plat.x + plat.width;
            }
            player->velocity.x = 0.0f;
            playerRectX.x = player->position.x;
        }
    }

    // Keep within world horizontal boundaries
    if (player->position.x < 0.0f) {
        player->position.x = 0.0f;
        player->velocity.x = 0.0f;
    } else if (player->position.x > WORLD_WIDTH - player->size.x) {
        player->position.x = WORLD_WIDTH - player->size.x;
        player->velocity.x = 0.0f;
    }

    // --- Y Axis Movement ---
    player->position.y += player->velocity.y * dt;
    Rectangle playerRectY = { player->position.x, player->position.y, player->size.x, player->size.y };

    bool foundFloor = false;
    bool landedThisFrame = false;

    for (int i = 0; i < world->platformCount; i++) {
        Rectangle plat = world->platforms[i].bounds;
        if (CheckAABB(playerRectY, plat)) {
            if (world->platforms[i].isDanger) {
                PlayerRespawn(player);
                return;
            }
            if (player->velocity.y > 0.0f) { // Landing on top of platform
                player->position.y = plat.y - player->size.y;
                player->velocity.y = 0.0f;
                foundFloor = true;
                if (!player->onGround) {
                    landedThisFrame = true;
                }
            } else if (player->velocity.y < 0.0f) { // Hitting ceiling
                player->position.y = plat.y + plat.height;
                player->velocity.y = 0.0f;
            }
            playerRectY.y = player->position.y;
        }
    }

    // Update Ground State
    if (foundFloor) {
        player->onGround = true;
        player->isJumping = false;
        if (landedThisFrame) {
            player->stretch = (Vector2){ 1.30f, 0.75f }; // Squash on landing
            Vector2 feetPos = { player->position.x + player->size.x * 0.5f, player->position.y + player->size.y };
            ParticleSpawnBurst(feetPos, 9, (Color){ 210, 210, 210, 220 }, 30.0f, 95.0f, 0.25f);
        }
    } else {
        player->onGround = false;
    }

    // 6. Smoothly return squash & stretch to normal (1.0)
    player->stretch.x += (1.0f - player->stretch.x) * 14.0f * dt;
    player->stretch.y += (1.0f - player->stretch.y) * 14.0f * dt;

    // 7. Check Collectibles Pickup
    Vector2 playerCenter = {
        player->position.x + player->size.x * 0.5f,
        player->position.y + player->size.y * 0.5f
    };
    for (int i = 0; i < world->collectibleCount; i++) {
        Collectible *c = &world->collectibles[i];
        if (!c->collected) {
            float dist = Vector2Distance(playerCenter, c->position);
            if (dist < c->radius + player->size.x * 0.5f) {
                c->collected = true;
                player->score += 100;
                ParticleSpawnBurst(c->position, 14, (Color){ 255, 225, 45, 255 }, 60.0f, 150.0f, 0.35f);
            }
        }
    }

    // 8. Check Goal Flag Reach
    Rectangle playerFullRect = { player->position.x, player->position.y, player->size.x, player->size.y };
    if (!world->goal.reached && CheckAABB(playerFullRect, world->goal.bounds)) {
        world->goal.reached = true;
        player->reachedGoal = true;
        Vector2 goalCenter = {
            world->goal.bounds.x + world->goal.bounds.width * 0.5f,
            world->goal.bounds.y + world->goal.bounds.height * 0.5f
        };
        ParticleSpawnBurst(goalCenter, 30, (Color){ 46, 204, 113, 255 }, 70.0f, 180.0f, 0.6f);
    }

    // 9. Falling into Pit -> Respawn
    if (player->position.y > WORLD_HEIGHT + 60.0f) {
        PlayerRespawn(player);
    }
}

void PlayerDraw(const Player *player, const GameAssets *assets) {
    // Calculate drawn rectangle taking squash & stretch into account (anchored at feet)
    float drawW = player->size.x * player->stretch.x;
    float drawH = player->size.y * player->stretch.y;
    float drawX = player->position.x + (player->size.x - drawW) * 0.5f;
    float drawY = player->position.y + (player->size.y - drawH);

    // Subtle drop shadow under character when on ground
    if (player->onGround) {
        DrawEllipse((int)(player->position.x + player->size.x * 0.5f),
                    (int)(player->position.y + player->size.y),
                    drawW * 0.45f, 4.0f, (Color){ 0, 0, 0, 60 });
    }

    // If custom sprite texture is loaded, draw texture with flip and squash/stretch
    if (assets && assets->hasPlayerTexture) {
        Rectangle sourceRec = {
            0.0f,
            0.0f,
            (float)assets->playerTexture.width * (float)player->facing,
            (float)assets->playerTexture.height
        };
        Rectangle destRec = { drawX, drawY, drawW, drawH };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(assets->playerTexture, sourceRec, destRec, origin, 0.0f, WHITE);
    } else {
        // Fallback: Plain solid blue rectangle
        DrawRectangleRec((Rectangle){ drawX, drawY, drawW, drawH }, (Color){ 59, 130, 246, 255 });
    }

    // Visual friction line when wall sliding
    if (player->isWallSliding && player->wallDirection != 0) {
        float edgeX = (player->wallDirection == 1) ? (player->position.x + player->size.x) : player->position.x;
        DrawLineEx((Vector2){ edgeX, player->position.y + 4.0f },
                   (Vector2){ edgeX, player->position.y + player->size.y - 4.0f },
                   2.0f, (Color){ 241, 196, 15, 200 });
    }
}
