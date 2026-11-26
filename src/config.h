#pragma once
#include <windows.h>
#include <cstdint>

struct ModConfig {
    // [General]
    bool enabled = true;
    int toggleKey = VK_F9;      // Toggle entire overlay
    int modeKey = VK_F10;       // Cycle bar mode: Custom Bar+Numbers -> Numbers Only -> Off
    bool enableLog = true;

    // [HealthBar]
    bool showHealthBar = true;
    bool drawCustomBar = true;  // 1 = draw sleek custom bar + numbers; 0 = numbers only (companion mode)
    bool hideVanillaBar = true; // When drawCustomBar=1, hide vanilla HudEnemyHp bar to prevent double bars
    bool showHpNumbers = true;
    bool showPercent = false;
    float barWidth = 112.0f;
    float barHeight = 9.0f;
    float barOffsetY = -18.0f;  // Screen-space Y offset above projected head position (negative = higher)
    float maxDistance = 35.0f;  // Meters
    float lingerSeconds = 4.0f; // Keep health bar visible after enemy takes damage
    float hpFontScale = 0.82f;

    // [DamageNumbers]
    bool showDamageNumbers = true;
    float dmgDuration = 1.15f;
    float dmgFloatSpeed = 62.0f;
    float dmgFontScale = 1.05f;
    float critHpRatio = 0.35f;  // Damage >= 35% of enemy maxHP highlights as heavy/crit
    float critMinDamage = 400.0f;
    float minDamage = 1.0f;     // Ignore tiny sub-1.0 rounding ticks
    bool showKillBadge = true;  // Append "!" on killing blows
};

extern ModConfig g_cfg;

void LogInit(HMODULE selfMod);
void LogPrintf(const char* fmt, ...);
void ConfigLoad(HMODULE selfMod);
