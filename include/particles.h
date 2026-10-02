#ifndef PARTICLES_H
#define PARTICLES_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_PARTICLES 128

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float life;
    float maxLife;
    bool active;
} Particle;

void ParticleSystemInit(void);
void ParticleSystemUpdate(float dt);
void ParticleSystemDraw(void);
void ParticleSpawn(Vector2 pos, Vector2 velocity, Color color, float size, float life);
void ParticleSpawnBurst(Vector2 pos, int count, Color color, float speedMin, float speedMax, float life);

#endif // PARTICLES_H
