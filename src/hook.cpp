#include "hook.h"
#include <cstring>

static void WriteAbsJump(uint8_t* dst, void* target) {
    // jmp qword ptr [rip+0]
    dst[0] = 0xFF;
    dst[1] = 0x25;
    dst[2] = 0x00;
    dst[3] = 0x00;
    dst[4] = 0x00;
    dst[5] = 0x00;
    uint64_t addr = reinterpret_cast<uint64_t>(target);
    memcpy(dst + 6, &addr, 8);
}

bool InstallInlineHook(InlineHook& hook, void* target, void* detour, size_t stolenLen) {
    if (!target || !detour || stolenLen < 14 || stolenLen > sizeof(hook.origBytes)) {
        return false;
    }

    uint8_t* tramp = static_cast<uint8_t*>(
        VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!tramp) return false;

    memcpy(hook.origBytes, target, stolenLen);
    memcpy(tramp, target, stolenLen);
    WriteAbsJump(tramp + stolenLen, static_cast<uint8_t*>(target) + stolenLen);

    DWORD oldProt = 0;
    if (!VirtualProtect(target, stolenLen, PAGE_EXECUTE_READWRITE, &oldProt)) {
        VirtualFree(tramp, 0, MEM_RELEASE);
        return false;
    }

    WriteAbsJump(static_cast<uint8_t*>(target), detour);
    for (size_t i = 14; i < stolenLen; ++i) {
        static_cast<uint8_t*>(target)[i] = 0x90; // NOP pad
    }

    DWORD dummy = 0;
    VirtualProtect(target, stolenLen, oldProt, &dummy);
    FlushInstructionCache(GetCurrentProcess(), target, stolenLen);
    FlushInstructionCache(GetCurrentProcess(), tramp, 64);

    hook.target = target;
    hook.trampoline = tramp;
    hook.stolenLen = stolenLen;
    hook.installed = true;
    return true;
}

void RemoveInlineHook(InlineHook& hook) {
    if (!hook.installed || !hook.target) return;
    DWORD oldProt = 0;
    if (VirtualProtect(hook.target, hook.stolenLen, PAGE_EXECUTE_READWRITE, &oldProt)) {
        memcpy(hook.target, hook.origBytes, hook.stolenLen);
        DWORD dummy = 0;
        VirtualProtect(hook.target, hook.stolenLen, oldProt, &dummy);
        FlushInstructionCache(GetCurrentProcess(), hook.target, hook.stolenLen);
    }
    if (hook.trampoline) {
        VirtualFree(hook.trampoline, 0, MEM_RELEASE);
        hook.trampoline = nullptr;
    }
    hook.installed = false;
}

bool PatchVTableSlot(void** vtable, size_t index, void* detour, void** outOrig) {
    if (!vtable || !detour) return false;
    DWORD oldProt = 0;
    if (!VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProt)) {
        return false;
    }
    if (outOrig) *outOrig = vtable[index];
    vtable[index] = detour;
    DWORD dummy = 0;
    VirtualProtect(&vtable[index], sizeof(void*), oldProt, &dummy);
    return true;
}
