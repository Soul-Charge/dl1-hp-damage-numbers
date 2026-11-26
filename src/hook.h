#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

struct InlineHook {
    void* target = nullptr;
    void* trampoline = nullptr;
    uint8_t origBytes[32] = {};
    size_t stolenLen = 0;
    bool installed = false;
};

// Installs an x64 14-byte absolute jump (FF 25 00 00 00 00 <abs64>) at target.
// stolenLen MUST be >= 14 and end on an instruction boundary with NO RIP-relative instructions in [0, stolenLen).
bool InstallInlineHook(InlineHook& hook, void* target, void* detour, size_t stolenLen);
void RemoveInlineHook(InlineHook& hook);

// Replaces a single vtable slot safely with VirtualProtect.
bool PatchVTableSlot(void** vtable, size_t index, void* detour, void** outOrig);
