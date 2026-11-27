#include "config.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>

ModConfig g_cfg;

static char g_modDir[MAX_PATH] = {};
static FILE* g_logFile = nullptr;
static CRITICAL_SECTION g_logCs;
static bool g_logCsInit = false;

static void EnsureModDir(HMODULE selfMod) {
    if (g_modDir[0] != '\0') return;
    GetModuleFileNameA(selfMod, g_modDir, MAX_PATH);
    char* slash = strrchr(g_modDir, '\\');
    if (slash) *(slash + 1) = '\0';
}

void LogInit(HMODULE selfMod) {
    EnsureModDir(selfMod);
    if (!g_logCsInit) {
        InitializeCriticalSection(&g_logCs);
        g_logCsInit = true;
    }
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%sdl1_hp_damage.log", g_modDir);
    g_logFile = fopen(path, "w");
    if (g_logFile) {
        fprintf(g_logFile, "DL1 Enemy HP & Damage Numbers v1.0\n");
        fflush(g_logFile);
    }
}

void LogPrintf(const char* fmt, ...) {
    if (!g_cfg.enableLog || !g_logFile) return;
    EnterCriticalSection(&g_logCs);
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(g_logFile, "[%02u:%02u:%02u.%03u] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_list args;
    va_start(args, fmt);
    vfprintf(g_logFile, fmt, args);
    va_end(args);
    fprintf(g_logFile, "\n");
    fflush(g_logFile);
    LeaveCriticalSection(&g_logCs);
}

static float ReadIniFloat(const char* sec, const char* key, float defVal, const char* path) {
    char buf[64] = {};
    char defStr[64];
    snprintf(defStr, sizeof(defStr), "%.3f", defVal);
    GetPrivateProfileStringA(sec, key, defStr, buf, sizeof(buf), path);
    char* end = nullptr;
    float v = strtof(buf, &end);
    return (end && end != buf) ? v : defVal;
}

static int ReadIniInt(const char* sec, const char* key, int defVal, const char* path) {
    char buf[64] = {};
    char defStr[64];
    snprintf(defStr, sizeof(defStr), "%d", defVal);
    GetPrivateProfileStringA(sec, key, defStr, buf, sizeof(buf), path);
    char* end = nullptr;
    long v = strtol(buf, &end, 0); // supports 0x78 hex and decimal
    return (end && end != buf) ? static_cast<int>(v) : defVal;
}

void ConfigLoad(HMODULE selfMod) {
    EnsureModDir(selfMod);
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%sdl1_hp_damage.ini", g_modDir);

    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        LogPrintf("Config not found at %s, using defaults.", path);
        return;
    }

    g_cfg.enabled           = ReadIniInt("General", "enabled", 1, path) != 0;
    g_cfg.toggleKey         = ReadIniInt("General", "toggle_key", VK_F7, path);
    g_cfg.modeKey           = ReadIniInt("General", "mode_key", VK_F8, path);
    g_cfg.enableLog         = ReadIniInt("General", "enable_log", 1, path) != 0;

    g_cfg.showHealthBar     = ReadIniInt("HealthBar", "show_health_bar", 1, path) != 0;
    g_cfg.drawCustomBar     = ReadIniInt("HealthBar", "draw_custom_bar", 1, path) != 0;
    g_cfg.hideVanillaBar    = ReadIniInt("HealthBar", "hide_vanilla_bar", 1, path) != 0;
    g_cfg.showHpNumbers     = ReadIniInt("HealthBar", "show_hp_numbers", 1, path) != 0;
    g_cfg.showPercent       = ReadIniInt("HealthBar", "show_percent", 0, path) != 0;
    g_cfg.barWidth          = ReadIniFloat("HealthBar", "bar_width", 112.0f, path);
    g_cfg.barHeight         = ReadIniFloat("HealthBar", "bar_height", 9.0f, path);
    g_cfg.barOffsetY        = ReadIniFloat("HealthBar", "bar_offset_y", -18.0f, path);
    g_cfg.maxDistance       = ReadIniFloat("HealthBar", "max_distance", 35.0f, path);
    g_cfg.lingerSeconds     = ReadIniFloat("HealthBar", "linger_seconds", 4.0f, path);
    g_cfg.hpFontScale       = ReadIniFloat("HealthBar", "hp_font_scale", 0.82f, path);

    g_cfg.showDamageNumbers = ReadIniInt("DamageNumbers", "show_damage_numbers", 1, path) != 0;
    g_cfg.dmgDuration       = ReadIniFloat("DamageNumbers", "duration", 1.15f, path);
    g_cfg.dmgFloatSpeed     = ReadIniFloat("DamageNumbers", "float_speed", 62.0f, path);
    g_cfg.dmgFontScale      = ReadIniFloat("DamageNumbers", "font_scale", 1.05f, path);
    g_cfg.critHpRatio       = ReadIniFloat("DamageNumbers", "crit_hp_ratio", 0.35f, path);
    g_cfg.critMinDamage     = ReadIniFloat("DamageNumbers", "crit_min_damage", 400.0f, path);
    g_cfg.minDamage         = ReadIniFloat("DamageNumbers", "min_damage", 1.0f, path);
    g_cfg.showKillBadge     = ReadIniInt("DamageNumbers", "show_kill_badge", 1, path) != 0;

    LogPrintf("Config loaded: enabled=%d customBar=%d hideVanilla=%d hpNum=%d dmgNum=%d",
              g_cfg.enabled, g_cfg.drawCustomBar, g_cfg.hideVanillaBar,
              g_cfg.showHpNumbers, g_cfg.showDamageNumbers);
}
