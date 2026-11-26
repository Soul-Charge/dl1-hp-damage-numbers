#include <windows.h>
#include <cstring>
#include "config.h"
#include "game_hooks.h"
#include "d3d11_renderer.h"

static HMODULE g_selfMod = nullptr;
extern void WinmmProxyInit();

static bool IsDyingLightProcess() {
    char exePath[MAX_PATH] = {};
    if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH)) return false;
    const char* name = strrchr(exePath, '\\');
    name = name ? (name + 1) : exePath;
    return _stricmp(name, "DyingLightGame.exe") == 0;
}

static DWORD WINAPI InitThread(LPVOID) {
    LogInit(g_selfMod);
    ConfigLoad(g_selfMod);

    LogPrintf("Waiting for game modules (gamedll, engine, rd3d11)...");
    for (int i = 0; i < 200; ++i) {
        if (GetModuleHandleA("gamedll_x64_rwdi.dll") &&
            GetModuleHandleA("engine_x64_rwdi.dll") &&
            GetModuleHandleA("rd3d11_x64_rwdi.dll")) {
            break;
        }
        Sleep(150);
    }

    // Brief settle so ReShade / crinloader finish their startup patches first.
    Sleep(800);

    bool gameOk = GameHooksInit();
    bool dxOk   = D3D11OverlayInit();
    LogPrintf("Init complete: gameHooks=%d d3d11Overlay=%d", gameOk, dxOk);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_selfMod = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        WinmmProxyInit();
        if (IsDyingLightProcess()) {
            HANDLE hThread = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
            if (hThread) CloseHandle(hThread);
        }
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        D3D11OverlayShutdown();
        GameHooksShutdown();
    }
    return TRUE;
}
