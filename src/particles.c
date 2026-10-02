#include "particles.h"
#include "raymath.h"
#include <stdlib.h>

static Particle particles[MAX_PARTICLES];

void ParticleSystemInit(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

void ParticleSpawn(Vector2 pos, Vector2 velocity, Color color, float size, float life) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].position = pos;
            particles[i].velocity = velocity;
            particles[i].color = color;
            particles[i].size = size;
            particles[i].life = life;
            particles[i].maxLife = life;
            particles[i].active = true;
            return;
        }
    }
}

void ParticleSpawnBurst(Vector2 pos, int count, Color color, float speedMin, float speedMax, float life) {
    for (int i = 0; i < count; i++) {
        float angle = (float)(rand() % 360) * DEG2RAD;
        float speed = speedMin + ((float)rand() / (float)RAND_MAX) * (speedMax - speedMin);
        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        float pSize = 3.0f + ((float)rand() / (float)RAND_MAX) * 3.0f;
        ParticleSpawn(pos, vel, color, pSize, life);
    }
}

void ParticleSystemUpdate(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        particles[i].life -= dt;
        if (particles[i].life <= 0.0f) {
            particles[i].active = false;
            continue;
        }

        particles[i].position.x += particles[i].velocity.x * dt;
        particles[i].position.y += particles[i].velocity.y * dt;
        particles[i].velocity.y += 300.0f * dt;
    }
}

void ParticleSystemDraw(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        float alphaRatio = particles[i].life / particles[i].maxLife;
        Color c = particles[i].color;
        c.a = (unsigned char)(255 * alphaRatio);

        float currentSize = particles[i].size * alphaRatio;
        DrawCircleV(particles[i].position, currentSize, c);
    }
}
