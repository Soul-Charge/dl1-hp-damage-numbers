#include "game_hooks.h"
#include "config.h"
#include "hook.h"
#include <unordered_map>
#include <cmath>
#include <cstring>
#include <algorithm>

// ---- Engine & GameDLL types ----
using FnGetCombinedMatrix = const Mtx44* (__fastcall*)(void* cam);
using FnGetCameraPos      = Vec3* (__fastcall*)(void* cam, Vec3* out);
using FnSetVisible        = void (__fastcall*)(void* uiObj, bool visible);
using FnTakeDamage        = void (__fastcall*)(void* controlObj, const void* dmgInfo);
using FnHudEnemyHpUpdate  = void (__fastcall*)(void* item, void* enemy, float curHP, float maxHP,
                                               float curArmor, float maxArmor, int level);

// ---- Known DL1 1.55 (Steam) RVAs in gamedll_x64_rwdi.dll ----
static constexpr uintptr_t RVA_HUD_ENEMY_HP_UPDATE = 0x5d0740;
static constexpr uintptr_t RVA_HUD_ENEMY_HP_ENABLE = 0x5d0d91;
static constexpr uintptr_t RVA_G_PGAME             = 0x1c16348;
static constexpr uintptr_t RVA_IAT_SET_VISIBLE     = 0x137c500;

static uint8_t* g_gameDllBase   = nullptr;
static uint8_t* g_engineDllBase = nullptr;

static FnGetCombinedMatrix g_fnGetCombinedMatrix = nullptr;
static FnGetCameraPos      g_fnGetCameraPos      = nullptr;
static FnSetVisible        g_fnSetVisible        = nullptr;

static InlineHook g_hookHudHp;
static InlineHook g_hookTakeDamage;

static SRWLOCK g_stateLock = SRWLOCK_INIT;
static std::unordered_map<uintptr_t, TrackedEnemy> g_enemies;
static std::vector<DamagePopup> g_popups;
static uint32_t g_rngState = 0x12345678u;

static double GetNowSeconds() {
    static LARGE_INTEGER freq = {};
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
    LARGE_INTEGER now = {};
    QueryPerformanceCounter(&now);
    return static_cast<double>(now.QuadPart) / static_cast<double>(freq.QuadPart);
}

static float NextRandFloat(float lo, float hi) {
    // Simple xorshift32
    uint32_t x = g_rngState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rngState = x;
    float t = static_cast<float>(x & 0xFFFFu) / 65535.0f;
    return lo + t * (hi - lo);
}

// ---- SEH-safe memory helpers ----
static bool SafeReadPtr(uintptr_t addr, uintptr_t& out) {
    if (addr < 0x10000 || addr > 0x00007FFFFFFFFFFFULL) return false;
    __try {
        out = *reinterpret_cast<uintptr_t*>(addr);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeReadFloat(uintptr_t addr, float& out) {
    if (addr < 0x10000 || addr > 0x00007FFFFFFFFFFFULL) return false;
    __try {
        out = *reinterpret_cast<float*>(addr);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeIsAI(void* controlObj) {
    if (!controlObj) return false;
    __try {
        uintptr_t vtbl = *reinterpret_cast<uintptr_t*>(controlObj);
        if (vtbl < 0x10000) return false;
        using FnIsAI = bool (__fastcall*)(void*);
        auto fn = *reinterpret_cast<FnIsAI*>(vtbl + 0x698);
        if (!fn) return false;
        return fn(controlObj);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Validates that enemy is a real CAI with an active HealthModule whose owner == enemy.
static bool SafeReadEnemyHealth(uintptr_t enemy, float& outCurHP, float& outMaxHP) {
    if (enemy < 0x10000) return false;
    __try {
        uintptr_t hm = *reinterpret_cast<uintptr_t*>(enemy + 0xce8);
        if (hm < 0x10000) return false;
        // HealthModule::owner back-pointer at +0x40 must match enemy
        uintptr_t owner = *reinterpret_cast<uintptr_t*>(hm + 0x40);
        if (owner != enemy) return false;
        // Cached module active flag at +0x4c must be 0.0f
        if (*reinterpret_cast<float*>(hm + 0x4c) != 0.0f) return false;
        float cur = *reinterpret_cast<float*>(hm + 0x78);
        float mx  = *reinterpret_cast<float*>(hm + 0x7c);
        if (!std::isfinite(cur) || !std::isfinite(mx) || mx <= 0.0f) return false;
        outCurHP = cur;
        outMaxHP = mx;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Reads the exact 3D head/bar position via (enemy + 0x18)->vtable[0x1c8](enemy + 0x18, &out)
static bool SafeGetEnemyHeadPos(uintptr_t enemy, Vec3& outPos) {
    if (enemy < 0x10000) return false;
    __try {
        void* subObj = reinterpret_cast<void*>(enemy + 0x18);
        uintptr_t vtbl = *reinterpret_cast<uintptr_t*>(subObj);
        if (vtbl < 0x10000) return false;
        using FnGetHead = Vec3* (__fastcall*)(void*, Vec3*);
        auto fn = *reinterpret_cast<FnGetHead*>(vtbl + 0x1c8);
        if (!fn) return false;
        Vec3 tmp = {0, 0, 0};
        Vec3* res = fn(subObj, &tmp);
        if (!res) return false;
        if (!std::isfinite(res->x) || !std::isfinite(res->y) || !std::isfinite(res->z)) return false;
        outPos = *res;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeGetActiveCamera(Mtx44& outVP, Vec3& outCamPos) {
    if (!g_gameDllBase || !g_fnGetCombinedMatrix) return false;
    __try {
        uintptr_t pGame = *reinterpret_cast<uintptr_t*>(g_gameDllBase + RVA_G_PGAME);
        if (pGame < 0x10000) return false;
        uintptr_t pLevel = *reinterpret_cast<uintptr_t*>(pGame + 0x540);
        if (pLevel < 0x10000) return false;
        uintptr_t pView = *reinterpret_cast<uintptr_t*>(pLevel + 0xc0);
        if (pView < 0x10000) return false;
        void* pCam = *reinterpret_cast<void**>(pView + 0x50);
        if (reinterpret_cast<uintptr_t>(pCam) < 0x10000) return false;

        const Mtx44* vp = g_fnGetCombinedMatrix(pCam);
        if (!vp) return false;
        memcpy(outVP.m, vp->m, sizeof(outVP.m));

        if (g_fnGetCameraPos) {
            Vec3 tmp = {0, 0, 0};
            Vec3* p = g_fnGetCameraPos(pCam, &tmp);
            if (p) outCamPos = *p;
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static void SafeHideVanillaWidgets(void* item) {
    if (!item || !g_fnSetVisible) return;
    __try {
        uintptr_t base = reinterpret_cast<uintptr_t>(item);
        void* wBar    = *reinterpret_cast<void**>(base + 0x48);
        void* wStam   = *reinterpret_cast<void**>(base + 0x58);
        void* wBg     = *reinterpret_cast<void**>(base + 0x80);
        void* wLevel  = *reinterpret_cast<void**>(base + 0x40);
        if (wBar)   g_fnSetVisible(wBar, false);
        if (wStam)  g_fnSetVisible(wStam, false);
        if (wBg)    g_fnSetVisible(wBg, false);
        if (wLevel) g_fnSetVisible(wLevel, false);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// ---- Detour #1: CHudEnemyHpItem::Update ----
static void __fastcall HkHudEnemyHpUpdate(void* item, void* enemyPtr, float curHP, float maxHP,
                                          float curArmor, float maxArmor, int level) {
    auto orig = reinterpret_cast<FnHudEnemyHpUpdate>(g_hookHudHp.trampoline);

    if (enemyPtr && maxHP > 0.0f && std::isfinite(curHP) && std::isfinite(maxHP)) {
        uintptr_t enemy = reinterpret_cast<uintptr_t>(enemyPtr);
        Vec3 pos = {0, 0, 0};
        if (SafeGetEnemyHeadPos(enemy, pos)) {
            double now = GetNowSeconds();
            AcquireSRWLockExclusive(&g_stateLock);
            auto& entry = g_enemies[enemy];
            if (entry.enemyPtr == 0) {
                entry.enemyPtr = enemy;
                entry.displayHP = curHP;
            }
            entry.curHP = (curHP < 0.0f) ? 0.0f : curHP;
            entry.maxHP = maxHP;
            if (entry.displayHP < entry.curHP) entry.displayHP = entry.curHP;
            entry.worldPos = pos;
            entry.lastAimedTime = now;
            entry.isAimed = true;
            if (entry.curHP <= 0.01f && !entry.isDead) {
                entry.isDead = true;
                entry.deathTime = now;
            } else if (entry.curHP > 0.01f) {
                entry.isDead = false;
                entry.deathTime = 0.0;
            }
            ReleaseSRWLockExclusive(&g_stateLock);
        }
    }

    orig(item, enemyPtr, curHP, maxHP, curArmor, maxArmor, level);

    if (g_cfg.enabled && g_cfg.showHealthBar && g_cfg.drawCustomBar && g_cfg.hideVanillaBar) {
        SafeHideVanillaWidgets(item);
    }
}

// ---- Detour #2: IControlObject::TakeDamage ----
static void __fastcall HkTakeDamage(void* controlObj, const void* dmgInfo) {
    auto orig = reinterpret_cast<FnTakeDamage>(g_hookTakeDamage.trampoline);

    bool isEnemyAI = false;
    uintptr_t enemy = 0;
    float hpBefore = 0.0f;
    float maxHPBefore = 0.0f;

    if (controlObj && SafeIsAI(controlObj)) {
        uintptr_t candidate = reinterpret_cast<uintptr_t>(controlObj) - 0x28;
        if (SafeReadEnemyHealth(candidate, hpBefore, maxHPBefore)) {
            isEnemyAI = true;
            enemy = candidate;
        }
    }

    orig(controlObj, dmgInfo);

    if (isEnemyAI && hpBefore > 0.01f) {
        float hpAfter = hpBefore;
        float maxHPAfter = maxHPBefore;
        // After a killing blow, HealthModule may still be readable with curHP == 0
        if (!SafeReadEnemyHealth(enemy, hpAfter, maxHPAfter)) {
            hpAfter = 0.0f;
        }
        if (hpAfter < 0.0f) hpAfter = 0.0f;

        float dealt = hpBefore - hpAfter;
        if (dealt >= g_cfg.minDamage) {
            Vec3 pos = {0, 0, 0};
            bool hasPos = SafeGetEnemyHeadPos(enemy, pos);
            double now = GetNowSeconds();

            AcquireSRWLockExclusive(&g_stateLock);
            auto& entry = g_enemies[enemy];
            if (entry.enemyPtr == 0) {
                entry.enemyPtr = enemy;
                entry.displayHP = hpBefore;
            }
            entry.curHP = hpAfter;
            entry.maxHP = (maxHPAfter > 0.0f) ? maxHPAfter : maxHPBefore;
            if (hasPos) entry.worldPos = pos;
            entry.lastDamageTime = now;
            if (hpAfter <= 0.01f) {
                entry.isDead = true;
                entry.deathTime = now;
            }

            if (g_cfg.showDamageNumbers && hasPos) {
                DamagePopup pop;
                pop.worldPos = pos;
                // Slightly below top of head (face/upper chest area) + small 3D jitter
                pop.worldPos.y -= 0.15f;
                pop.worldPos.x += NextRandFloat(-0.18f, 0.18f);
                pop.worldPos.z += NextRandFloat(-0.18f, 0.18f);
                pop.amount = dealt;
                pop.age = 0.0f;
                pop.duration = g_cfg.dmgDuration;
                pop.screenOffsetX = NextRandFloat(-14.0f, 14.0f);
                pop.screenVelX = NextRandFloat(-22.0f, 22.0f);
                pop.screenOffsetY = 0.0f;
                pop.isKill = (hpAfter <= 0.01f);
                pop.isCrit = (entry.maxHP > 0.0f && dealt >= entry.maxHP * g_cfg.critHpRatio)
                             || (dealt >= g_cfg.critMinDamage);

                if (g_popups.size() >= 64) {
                    g_popups.erase(g_popups.begin());
                }
                g_popups.push_back(pop);
            }
            ReleaseSRWLockExclusive(&g_stateLock);
        }
    }
}

// Enables vanilla HudEnemyHp layer outside Be the Zombie if not already patched by crinloader.
static void EnableHudEnemyHpIfVanilla() {
    uint8_t* addr = g_gameDllBase + RVA_HUD_ENEMY_HP_ENABLE;
    // Vanilla DL 1.55 bytes at +0x5d0d91:
    //   41 f6 47 0c 20   test byte ptr [r15+0xc], 0x20
    //   75 4a            jne  +0x4a
    const uint8_t expected[7] = {0x41, 0xF6, 0x47, 0x0C, 0x20, 0x75, 0x4A};
    if (memcmp(addr, expected, sizeof(expected)) == 0) {
        DWORD oldProt = 0;
        if (VirtualProtect(addr, 7, PAGE_EXECUTE_READWRITE, &oldProt)) {
            addr[0] = 0x90;
            addr[1] = 0x90;
            addr[2] = 0x90;
            addr[3] = 0x90;
            addr[4] = 0x90;
            addr[5] = 0xEB; // jmp +0x4a (always show HudEnemyHp)
            DWORD dummy = 0;
            VirtualProtect(addr, 7, oldProt, &dummy);
            FlushInstructionCache(GetCurrentProcess(), addr, 7);
            LogPrintf("Enabled HudEnemyHp layer at gamedll+0x%x (standalone mode).", (unsigned)RVA_HUD_ENEMY_HP_ENABLE);
        }
    } else {
        LogPrintf("HudEnemyHp layer at gamedll+0x%x already modified (e.g. crinloader) - leaving intact.",
                  (unsigned)RVA_HUD_ENEMY_HP_ENABLE);
    }
}

bool GameHooksInit() {
    HMODULE hGame = GetModuleHandleA("gamedll_x64_rwdi.dll");
    HMODULE hEngine = GetModuleHandleA("engine_x64_rwdi.dll");
    if (!hGame || !hEngine) {
        LogPrintf("GameHooksInit failed: hGame=%p hEngine=%p", hGame, hEngine);
        return false;
    }

    g_gameDllBase = reinterpret_cast<uint8_t*>(hGame);
    g_engineDllBase = reinterpret_cast<uint8_t*>(hEngine);

    // 1. Resolve engine exports
    g_fnGetCombinedMatrix = reinterpret_cast<FnGetCombinedMatrix>(
        GetProcAddress(hEngine, "?GetCombinedMatrix@IBaseCamera@@QEAAAEBVmtx44@@XZ"));
    g_fnGetCameraPos = reinterpret_cast<FnGetCameraPos>(
        GetProcAddress(hEngine, "?GetPosition@IBaseCamera@@QEBA?BVvec3@@XZ"));

    void* pTakeDamage = reinterpret_cast<void*>(
        GetProcAddress(hEngine, "?TakeDamage@IControlObject@@UEAAXAEBUSDamageInfo@@@Z"));

    LogPrintf("Engine exports: GetCombinedMatrix=%p GetPosition=%p TakeDamage=%p",
              g_fnGetCombinedMatrix, g_fnGetCameraPos, pTakeDamage);

    // Resolve SetVisible from gamedll IAT slot
    uintptr_t svPtr = 0;
    if (SafeReadPtr(reinterpret_cast<uintptr_t>(g_gameDllBase + RVA_IAT_SET_VISIBLE), svPtr) && svPtr > 0x10000) {
        g_fnSetVisible = reinterpret_cast<FnSetVisible>(svPtr);
    }

    // 2. Enable HudEnemyHp if vanilla (no crinloader)
    EnableHudEnemyHpIfVanilla();

    // 3. Hook CHudEnemyHpItem::Update (verify exact 15-byte prologue)
    uint8_t* pHudUpdate = g_gameDllBase + RVA_HUD_ENEMY_HP_UPDATE;
    const uint8_t expectedHud[15] = {
        0x48, 0x8B, 0xC4,
        0x48, 0x89, 0x58, 0x18,
        0x48, 0x89, 0x70, 0x20,
        0x55, 0x57, 0x41, 0x54
    };
    if (memcmp(pHudUpdate, expectedHud, sizeof(expectedHud)) == 0) {
        if (InstallInlineHook(g_hookHudHp, pHudUpdate, &HkHudEnemyHpUpdate, 15)) {
            LogPrintf("Hooked CHudEnemyHpItem::Update at %p", pHudUpdate);
        } else {
            LogPrintf("ERROR: Failed to hook CHudEnemyHpItem::Update");
        }
    } else {
        LogPrintf("WARN: CHudEnemyHpItem::Update prologue mismatch (unsupported game version?)");
    }

    // 4. Hook IControlObject::TakeDamage (verify exact 17-byte prologue)
    if (pTakeDamage) {
        const uint8_t expectedTD[17] = {
            0x40, 0x57,
            0x48, 0x83, 0xEC, 0x20,
            0x48, 0x8B, 0x41, 0x08,
            0x48, 0x8B, 0xF9,
            0x48, 0x8B, 0x48, 0x40
        };
        if (memcmp(pTakeDamage, expectedTD, sizeof(expectedTD)) == 0) {
            if (InstallInlineHook(g_hookTakeDamage, pTakeDamage, &HkTakeDamage, 17)) {
                LogPrintf("Hooked IControlObject::TakeDamage at %p", pTakeDamage);
            } else {
                LogPrintf("ERROR: Failed to hook IControlObject::TakeDamage");
            }
        } else {
            LogPrintf("WARN: IControlObject::TakeDamage prologue mismatch");
        }
    }

    return g_hookHudHp.installed || g_hookTakeDamage.installed;
}

void GameHooksShutdown() {
    RemoveInlineHook(g_hookTakeDamage);
    RemoveInlineHook(g_hookHudHp);
    AcquireSRWLockExclusive(&g_stateLock);
    g_enemies.clear();
    g_popups.clear();
    ReleaseSRWLockExclusive(&g_stateLock);
}

void GameHooksUpdateAndSnapshot(float dt, FrameSnapshot& out) {
    out.hasCamera = SafeGetActiveCamera(out.viewProj, out.camPos);
    out.enemies.clear();
    out.popups.clear();

    double now = GetNowSeconds();
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.1f) dt = 0.1f;

    AcquireSRWLockExclusive(&g_stateLock);

    // Update & prune tracked enemies
    for (auto it = g_enemies.begin(); it != g_enemies.end(); ) {
        auto& e = it->second;
        bool recentlyAimed = (now - e.lastAimedTime) < 0.28;
        bool recentlyHit   = (now - e.lastDamageTime) < static_cast<double>(g_cfg.lingerSeconds);
        e.isAimed = recentlyAimed;

        // Prune dead enemies after 0.65s fade, or untracked enemies after timeout
        if ((e.isDead && (now - e.deathTime) > 0.65) || (!recentlyAimed && !recentlyHit)) {
            it = g_enemies.erase(it);
            continue;
        }

        // Refresh live HP & head position for living recently-damaged enemies
        if (!e.isDead && !recentlyAimed) {
            float c = 0.0f, m = 0.0f;
            if (SafeReadEnemyHealth(e.enemyPtr, c, m)) {
                e.curHP = (c < 0.0f) ? 0.0f : c;
                e.maxHP = m;
                if (e.curHP <= 0.01f) {
                    e.isDead = true;
                    e.deathTime = now;
                } else {
                    Vec3 p;
                    if (SafeGetEnemyHeadPos(e.enemyPtr, p)) {
                        e.worldPos = p;
                    }
                }
            } else {
                // Entity despawned or invalid
                it = g_enemies.erase(it);
                continue;
            }
        }

        // Smoothly drain trailing displayHP toward curHP
        if (e.displayHP > e.curHP) {
            float drainSpeed = (std::max)(e.maxHP * 0.65f, (e.displayHP - e.curHP) * 4.5f);
            e.displayHP -= drainSpeed * dt;
            if (e.displayHP < e.curHP) e.displayHP = e.curHP;
        } else {
            e.displayHP = e.curHP;
        }

        out.enemies.push_back(e);
        ++it;
    }

    // Update & prune floating damage popups
    for (auto it = g_popups.begin(); it != g_popups.end(); ) {
        it->age += dt;
        if (it->age >= it->duration) {
            it = g_popups.erase(it);
            continue;
        }
        // Smooth upward float (decelerating slightly)
        float t = it->age / it->duration;
        float speedFactor = 1.0f - 0.55f * t;
        it->screenOffsetY -= g_cfg.dmgFloatSpeed * speedFactor * dt;
        it->screenOffsetX += it->screenVelX * (1.0f - t) * dt;
        out.popups.push_back(*it);
        ++it;
    }

    ReleaseSRWLockExclusive(&g_stateLock);
}

bool WorldToScreen(const Mtx44& vp, const Vec3& world, float screenW, float screenH, float& outX, float& outY) {
    float cx = vp.m[0]  * world.x + vp.m[1]  * world.y + vp.m[2]  * world.z + vp.m[3];
    float cy = vp.m[4]  * world.x + vp.m[5]  * world.y + vp.m[6]  * world.z + vp.m[7];
    float cw = vp.m[12] * world.x + vp.m[13] * world.y + vp.m[14] * world.z + vp.m[15];

    if (cw <= 0.05f) return false;

    float invW = 1.0f / cw;
    float ndcX = cx * invW;
    float ndcY = cy * invW;

    if (ndcX < -1.25f || ndcX > 1.25f || ndcY < -1.25f || ndcY > 1.25f) return false;

    outX = (ndcX * 0.5f + 0.5f) * screenW;
    outY = (0.5f - ndcY * 0.5f) * screenH;
    return true;
}
