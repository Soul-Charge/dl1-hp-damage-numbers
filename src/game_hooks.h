#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>

struct Vec3 {
    float x, y, z;
};

struct Mtx44 {
    float m[16];
};

struct TrackedEnemy {
    uintptr_t enemyPtr = 0;
    float curHP = 0.0f;
    float maxHP = 0.0f;
    float displayHP = 0.0f;     // Smoothly trails curHP for damage-drain animation
    Vec3 worldPos = {0, 0, 0};  // 3D head/bar anchor
    double lastAimedTime = 0.0;
    double lastDamageTime = 0.0;
    double lastPopupTime = 0.0; // Dedup between TakeDamage hook and HP-delta fallback
    double deathTime = 0.0;     // Non-zero once HP <= 0
    bool isAimed = false;
    bool isDead = false;
};

struct DamagePopup {
    Vec3 worldPos = {0, 0, 0};
    float amount = 0.0f;
    float age = 0.0f;
    float duration = 1.15f;
    float screenOffsetX = 0.0f;
    float screenVelX = 0.0f;
    float screenOffsetY = 0.0f;
    bool isCrit = false;
    bool isKill = false;
};

struct FrameSnapshot {
    bool hasCamera = false;
    Mtx44 viewProj = {};
    Vec3 camPos = {0, 0, 0};
    std::vector<TrackedEnemy> enemies;
    std::vector<DamagePopup> popups;
};

bool GameHooksInit();
void GameHooksShutdown();

// Called once per frame on the render thread (Present) to advance animations & get a thread-safe snapshot.
void GameHooksUpdateAndSnapshot(float dt, FrameSnapshot& out);

// Helper: project 3D world position using Mtx44 to screen pixels.
bool WorldToScreen(const Mtx44& vp, const Vec3& world, float screenW, float screenH, float& outX, float& outY);
