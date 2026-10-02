#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <stdbool.h>

#define SCREEN_WIDTH 800.0f
#define SCREEN_HEIGHT 450.0f

#define WORLD_WIDTH 2600.0f
#define WORLD_HEIGHT 600.0f

// Physics & Movement parameters
#define GRAVITY 1300.0f
#define GRAVITY_LIMIT 850.0f
#define JUMP_FORCE 520.0f
#define MOVE_SPEED 320.0f

#define GROUND_ACCEL 2200.0f
#define GROUND_FRICTION 1800.0f
#define AIR_ACCEL 1300.0f
#define AIR_DRAG 400.0f

// Platformer Feel / Juice parameters
#define COYOTE_TIME 0.12f          // Grace period to jump after leaving a ledge
#define JUMP_BUFFER_TIME 0.12f     // Early jump input queued before landing
#define JUMP_RELEASE_DAMPING 0.45f // Damping vertical speed when jump key is released early

#endif // COMMON_H
