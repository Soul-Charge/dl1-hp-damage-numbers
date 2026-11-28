====================================================================
  Enemy Health Bar Numbers & Floating Damage Numbers (v1.0.0)
  for Dying Light 1 (Steam 1.55, x64)
====================================================================

INSTALLATION
------------
1. Copy BOTH files into your Dying Light root folder (next to DyingLightGame.exe):
      - winmm.dll
      - dl1_hp_damage.ini
   Example path:
      ...\steamapps\common\Dying Light\

2. Launch the game! No .pak edits or mod managers required.

UNINSTALLATION
--------------
Delete winmm.dll, dl1_hp_damage.ini, and dl1_hp_damage.log from your
Dying Light root folder.

IN-GAME HOTKEYS (customizable in dl1_hp_damage.ini)
---------------------------------------------------
  F5 : Toggle the entire overlay ON / OFF
  F6 : Cycle bar mode:
         1. Custom RPG Health Bar + Exact HP Numbers (default)
         2. Exact HP Numbers Only (companion mode for vanilla / crinloader / 7px bars)
         3. Floating Damage Numbers Only

COMPATIBILITY
-------------
- Works standalone (no crinloader, no ReShade, no .pak edits required).
- Compatible with ReShade (dxgi.dll), crinloader (dsound.dll), DLMM, and all .pak mods.
- Uses a clean winmm.dll proxy that dynamically forwards all System32\winmm.dll exports.

IMPORTANT: MULTIPLAYER / VAC NOTICE
-----------------------------------
This mod hooks game memory and DirectX 11 rendering. Use it in single-player
or private co-op with VAC disabled. Do NOT use on VAC-secured public servers.

SOURCE CODE & LICENSE
---------------------
Open-source under the MIT License:
https://github.com/Soul-Charge/dl1-hp-damage-numbers
