# Nexus Mods 发布文案（直接复制粘贴）

> 以下内容按 Nexus Mods 后台字段整理好，**BBCode 正文**可直接全选粘贴到 Nexus Mods 的 `Description` 编辑框（切换为 BBCode 模式）。

---

## 1. Mod Name（模组标题）

```text
Enemy Health Bar Numbers and Floating Damage Numbers
```

## 2. Short Summary（简介，250 字符内）

```text
Displays exact enemy HP numbers (HP / MaxHP) on sleek RPG health bars and animated floating damage numbers on hit. Zero death-lag freeze, standalone DX11 overlay, compatible with ReShade, crinloader, and all .pak mods.
```

## 3. Suggested Category & Tags（建议分类与标签）

- **Category**: `User Interface`
- **Version**: `1.0.0`
- **Tags**: `UI`, `HUD`, `Health Bar`, `Damage Numbers`, `RPG`, `Quality of Life`, `DLL`, `Open Source`

---

## 4. Description（Nexus Mods BBCode 正文，直接复制）

```bbcode
[size=4][b]Enemy Health Bar Numbers & Floating Damage Numbers[/b][/size]

A lightweight, native DirectX 11 overlay mod for [b]Dying Light (Steam 1.55, 64-bit)[/b] that adds [b]exact enemy HP numbers[/b] alongside a clean RPG health bar and [b]real-time floating damage numbers[/b] whenever you hit an enemy.

No [font=Courier New].pak[/font] edits, no ReShade dependency, and no conflicts with [font=Courier New]crinloader[/font] or [font=Courier New]ReShade[/font].

[line]
[size=3][b]Features[/b][/size]
[list]
[*][b]Enemy Health Bars + Exact HP Numbers:[/b] Shows current and maximum health (e.g. [font=Courier New]1989 / 4091[/font], with optional percentage) directly above enemies you aim at or recently damaged.
[*][b]Zero Death-Lag / No Frozen Bars:[/b] Unlike vanilla [font=Courier New]HudEnemyHp[/font] bars that freeze at partial or full HP when an enemy dies in 1–2 shots, this mod syncs directly with the engine's [font=Courier New]IsDead()[/font] state every frame and drops the bar to [font=Courier New]0[/font] immediately on death.
[*][b]Animated Floating Damage Numbers:[/b] Pops crisp outlined damage numbers from enemies on melee, gunfire, arrows, explosions, and DoT ticks:
  [list]
  [*][color=#ffffff][b]White[/b][/color] — Normal hits
  [*][color=#ffd23f][b]Gold (enlarged)[/b][/color] — Heavy / critical hits (≥ 35% of enemy Max HP or ≥ 400 damage, configurable)
  [*][color=#ff453a][b]Red with "!" (enlarged)[/b][/color] — Killing blows
  [/list]
[*][b]Delayed Damage-Drain Trail:[/b] Smooth amber drain trail behind the green/yellow/red health bar fill, with distance scaling and fade-out.
[*][b]Companion Mode Built-In:[/b] Prefer your existing vanilla / crinloader / 7px health bar? Press [b]F6[/b] (or set [font=Courier New]draw_custom_bar=0[/font]) to display [b]only[/b] the crisp HP numbers + floating damage numbers.
[*][b]Multi-Target Tracking:[/b] Cleave attacks, grenades, and Molotovs track and display bars & damage numbers for all damaged enemies simultaneously.
[/list]

[line]
[size=3][b]In-Game Hotkeys[/b][/size]
[list]
[*][b]F5[/b] — Toggle the entire overlay ON / OFF (shows a brief status toast in the top-left corner).
[*][b]F6[/b] — Cycle display mode live in-game:
  [list=1]
  [*][b]Custom Health Bar + HP Numbers[/b] (default, automatically hides vanilla bar to prevent overlap)
  [*][b]HP Numbers Only[/b] (companion mode for vanilla / crinloader / 7px bars)
  [*][b]Damage Numbers Only[/b]
  [/list]
[/list]
All hotkeys, bar dimensions, colors/thresholds, font scales, distance limits, and linger durations can be customized in [font=Courier New]dl1_hp_damage.ini[/font].

[line]
[size=3][b]Installation & Uninstallation[/b][/size]
[b]Install:[/b]
[list=1]
[*]Download and extract the archive.
[*]Copy [font=Courier New]winmm.dll[/font] and [font=Courier New]dl1_hp_damage.ini[/font] into your Dying Light root folder (next to [font=Courier New]DyingLightGame.exe[/font]).
[*]Launch the game.
[/list]

[b]Uninstall:[/b]
Delete [font=Courier New]winmm.dll[/font], [font=Courier New]dl1_hp_damage.ini[/font], and [font=Courier New]dl1_hp_damage.log[/font] from your Dying Light root folder.

[line]
[size=3][b]Compatibility[/b][/size]
[list]
[*][b]Standalone:[/b] Works out of the box with no other mods required.
[*][b]ReShade ([font=Courier New]dxgi.dll[/font]):[/b] Fully compatible (tested alongside ReShade 6.8.0 + DX11 addons).
[*][b]crinloader ([font=Courier New]dsound.dll[/font]):[/b] Fully compatible — uses [font=Courier New]winmm.dll[/font] proxy, zero filename conflict.
[*][b]DLMM & [font=Courier New].pak[/font] mods:[/b] 100% compatible with all weapon, gameplay, and UI [font=Courier New].pak[/font] mods.
[*][b]Game Version:[/b] Built and tested on [b]Dying Light 1.55 (Steam, 64-bit)[/b]. Prologue byte checks and SEH guards prevent crashes on unsupported versions.
[/list]

[line]
[size=3][b]Important Notice (VAC / Multiplayer)[/b][/size]
[color=#ff6b6b][b]Single-player or private co-op with VAC disabled ONLY.[/b][/color]
Because this mod hooks game memory and DirectX 11 rendering, do [b]not[/b] use it in VAC-secured public games or Be the Zombie.

[line]
[size=3][b]Open Source & Transparency[/b][/size]
This mod is 100% open-source under the MIT License so anyone can inspect or compile it from source:
[url=https://github.com/Soul-Charge/dl1-hp-damage-numbers]GitHub Repository (Soul-Charge/dl1-hp-damage-numbers)[/url]
```

---

## 5. Changelog（更新日志）

```text
v1.0.0
- Initial release for Dying Light 1.55 (Steam x64).
- Added sleek DX11 custom enemy health bars with damage-drain trail and exact HP / MaxHP numbers.
- Added animated 3D-anchored floating damage numbers (white normal, gold heavy/crit, red killing blow with "!").
- Added per-frame AI IsDead() sync to eliminate the vanilla frozen health bar bug on 1-shot / 2-shot kills.
- Added F5 (toggle overlay) and F6 (cycle bar mode: Custom Bar + HP / HP Numbers Only / Damage Numbers Only) live hotkeys and full INI configuration.
```

---

## 6. N 网发布前清单（给你的提示）

1. **上传文件**：直接上传 `release/DL1-Enemy-HP-And-Damage-Numbers-v1.0.0.zip`（内含 `winmm.dll`、`dl1_hp_damage.ini`、`README.txt`、`LICENSE`）。
2. **截图建议**（N 网至少需要 1 张主图）：
   - 建议截 2–3 张实机图：① 瞄准丧尸/夜魔时的血条 + 精确血量数字；② 挥刀/开枪命中瞬间的伤害跳字（含金色高伤或红色击杀 `!`）；③ 按 `F6` 切换后的伴侣模式对比图。
3. **DLL 信任度加分项**：
   - 把 `winmm.dll` 传一次 [VirusTotal](https://www.virustotal.com/) 并把扫描结果链接贴到说明里（或者依靠 Nexus Mods 自带的 VirusTotal 自动扫描），配合上面的 GitHub 开源地址，老外玩家会非常放心下载。
