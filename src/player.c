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
    player->facing = 1;
    player->stretch = (Vector2){ 1.0f, 1.0f };
    player->score = 0;
    player->reachedGoal = false;
}

void PlayerRespawn(Player *player) {
    player->position = player->respawnPos;
    player->velocity = (Vector2){ 0.0f, 0.0f };
    player->onGround = false;
    player->isJumping = false;
    player->coyoteTimer = 0.0f;
    player->jumpBufferTimer = 0.0f;
    player->stretch = (Vector2){ 1.0f, 1.0f };
    ParticleSpawnBurst(player->position, 16, (Color){ 64, 150, 255, 255 }, 50.0f, 120.0f, 0.4f);
}

void PlayerUpdate(Player *player, World *world, float dt) {
    // 1. Horizontal Input & Acceleration / Deceleration
    int moveInput = 0;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInput += 1;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  moveInput -= 1;

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

    // 2. Vertical Physics (Gravity & Limits)
    player->velocity.y += GRAVITY * dt;
    if (player->velocity.y > GRAVITY_LIMIT) {
        player->velocity.y = GRAVITY_LIMIT;
    }

    // 3. Coyote Time & Jump Buffering
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

    // Execute Jump if buffered and allowed by ground or coyote timer
    if (player->jumpBufferTimer > 0.0f && player->coyoteTimer > 0.0f) {
        player->velocity.y = -JUMP_FORCE;
        player->onGround = false;
        player->isJumping = true;
        player->coyoteTimer = 0.0f;
        player->jumpBufferTimer = 0.0f;
        player->stretch = (Vector2){ 0.75f, 1.30f }; // Vertical stretch

        // Jump dust burst at feet
        Vector2 feetPos = { player->position.x + player->size.x * 0.5f, player->position.y + player->size.y };
        ParticleSpawnBurst(feetPos, 7, (Color){ 220, 220, 220, 220 }, 25.0f, 85.0f, 0.28f);
    }

    // 4. Variable Jump Height: Cut upward velocity early on key release
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
        // Fallback: Character body stylish vibrant rounded rectangle
        Rectangle charRect = { drawX, drawY, drawW, drawH };
        DrawRectangleRounded(charRect, 0.35f, 6, (Color){ 41, 128, 185, 255 }); // Main body blue
        DrawRectangleRoundedLines(charRect, 0.35f, 6, (Color){ 52, 152, 219, 255 }); // Bright border

        // Character face / visor based on facing direction
        float visorWidth = drawW * 0.45f;
        float visorHeight = drawH * 0.22f;
        float visorX = (player->facing == 1)
            ? (drawX + drawW * 0.5f)
            : (drawX + drawW * 0.08f);
        float visorY = drawY + drawH * 0.22f;

        Rectangle visorRect = { visorX, visorY, visorWidth, visorHeight };
        DrawRectangleRounded(visorRect, 0.4f, 4, (Color){ 236, 240, 241, 255 }); // Light visor
        // Visor glow dot
        float eyeX = (player->facing == 1) ? (visorX + visorWidth - 5.0f) : (visorX + 5.0f);
        DrawCircle((int)eyeX, (int)(visorY + visorHeight * 0.5f), 2.5f, (Color){ 46, 204, 113, 255 });
    }
}
