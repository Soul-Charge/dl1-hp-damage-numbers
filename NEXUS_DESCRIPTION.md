# Nexus Mods 双语发布文案（v1.0.1，直接复制粘贴）

## 1. Mod Name
```text
Enemy Health Bar Numbers and Floating Damage Numbers
```

## 2. Short Summary (<= 250 chars)
```text
Displays exact enemy HP numbers (HP/MaxHP) on sleek RPG health bars & animated floating damage numbers. Zero death-lag freeze, built-in EN/中文 support, standalone DX11 overlay compatible with ReShade, crinloader & all .pak mods.
```

## 3. Description (Nexus Mods Bilingual BBCode — English + 简体中文)

```bbcode
[size=4][b]Enemy Health Bar Numbers & Floating Damage Numbers[/b][/size]
[size=3][b]消逝的光芒 1 — 敌人血条精确数值 & 攻击伤害飘字模组（中英双语内置）[/b][/size]

A lightweight, native DirectX 11 overlay mod for [b]Dying Light (Steam 1.55, 64-bit)[/b] that adds [b]exact enemy HP numbers[/b] alongside a clean RPG health bar and [b]real-time floating damage numbers[/b] whenever you hit an enemy.
[i]Built-in English & Simplified Chinese support (auto-detects your Windows language, or configure [font=Courier New]language=en/zh[/font] in INI).[/i]

[i]中文说明请向下滚动查看 ↓[/i]

[line]
[size=4][b]English[/b][/size]

[size=3][b]Features[/b][/size]
[list]
[*][b]Enemy Health Bars + Exact HP Numbers:[/b] Shows current and maximum health (e.g. [font=Courier New]1989 / 4091[/font], with optional percentage) directly above enemies you aim at or recently damaged.
[*][b]Zero Death-Lag / No Frozen Bars:[/b] Unlike vanilla [font=Courier New]HudEnemyHp[/font] bars that freeze at partial or full HP when an enemy dies in 1–2 shots, this mod syncs directly with the engine's [font=Courier New]IsDead()[/font] state every frame and drops the bar to [font=Courier New]0[/font] immediately on death.
[*][b]Animated Floating Damage Numbers:[/b] Pops crisp outlined damage numbers from enemies on melee, gunfire, arrows, explosions, and DoT ticks:
  [list]
  [*][color=#ffffff][b]White[/b][/color] — Normal hits
  [*][color=#ffd23f][b]Gold (enlarged)[/b][/color] — Heavy / critical hits (>= 35% of enemy Max HP or >= 400 damage, configurable)
  [*][color=#ff453a][b]Red with "!" (enlarged)[/b][/color] — Killing blows
  [/list]
[*][b]Delayed Damage-Drain Trail:[/b] Smooth amber drain trail behind the green/yellow/red health bar fill, with distance scaling and fade-out.
[*][b]Companion Mode Built-In:[/b] Prefer your existing vanilla / crinloader / 7px health bar? Press [b]F6[/b] (or set [font=Courier New]draw_custom_bar=0[/font]) to display [b]only[/b] the crisp HP numbers + floating damage numbers.
[*][b]Multi-Target Tracking:[/b] Cleave attacks, grenades, and Molotovs track and display bars & damage numbers for all damaged enemies simultaneously.
[*][b]Built-In EN / 中文 Support:[/b] Automatically displays in-game status toasts in English or Simplified Chinese based on Windows UI language (configurable via [font=Courier New]language=auto|en|zh[/font]).
[/list]

[size=3][b]In-Game Hotkeys[/b][/size]
[list]
[*][b]F5[/b] — Toggle the entire overlay ON / OFF.
[*][b]F6[/b] — Cycle display mode live in-game:
  [list=1]
  [*][b]Custom Health Bar + HP Numbers[/b] (default, automatically hides vanilla bar to prevent overlap)
  [*][b]HP Numbers Only[/b] (companion mode for vanilla / crinloader / 7px bars)
  [*][b]Damage Numbers Only[/b]
  [/list]
[/list]
All hotkeys, bar dimensions, thresholds, font scales, distance limits, and linger durations can be customized in [font=Courier New]dl1_hp_damage.ini[/font].

[size=3][b]Installation & Uninstallation[/b][/size]
[b]Install:[/b] Extract [font=Courier New]winmm.dll[/font] and [font=Courier New]dl1_hp_damage.ini[/font] into your Dying Light root folder (next to [font=Courier New]DyingLightGame.exe[/font]).
[b]Uninstall:[/b] Delete [font=Courier New]winmm.dll[/font], [font=Courier New]dl1_hp_damage.ini[/font], and [font=Courier New]dl1_hp_damage.log[/font].

[size=3][b]Compatibility & VAC Notice[/b][/size]
[list]
[*]Standalone — no [font=Courier New].pak[/font] edits, no ReShade or crinloader required.
[*]100% compatible with [b]ReShade[/b] ([font=Courier New]dxgi.dll[/font]), [b]crinloader[/b] ([font=Courier New]dsound.dll[/font]), [b]DLMM[/b], and all [font=Courier New].pak[/font] mods (uses [font=Courier New]winmm.dll[/font] proxy).
[*]Tested on [b]Dying Light 1.55 (Steam, 64-bit)[/b].
[*][color=#ff6b6b][b]VAC Notice:[/b][/color] Single-player or private co-op with VAC disabled ONLY. Do not use on VAC-secured public servers.
[/list]

[line]
[size=4][b]简体中文说明[/b][/size]

专为《消逝的光芒 1》（Steam 1.55 64位）打造的原生 DirectX 11 覆盖层模组，为敌人血条加上[b]精确血量数字[/b]并在攻击命中时实时弹出[b]伤害跳字[/b]。模组内置中英双语支持，默认根据系统语言自动切换（也可在配置文件中手动指定 [font=Courier New]language=zh[/font] 或 [font=Courier New]en[/font]）。

[size=3][b]主要特性[/b][/size]
[list]
[*][b]敌人血条 + 精确血量数字：[/b]在瞄准或近期受击的敌人头顶实时显示当前/最大血量（例如 [font=Courier New]1989 / 4091[/font]，可选附带百分比）。
[*][b]彻底消除死后残血/满血定格：[/b]原版 [font=Courier New]HudEnemyHp[/font] 在一两枪秒杀敌人时会定格在满血或残血；本模组每帧直连引擎 [font=Courier New]IsDead()[/font] 状态，敌人死亡瞬间血条立刻归零淡出。
[*][b]攻击伤害飘字：[/b]近战、枪械、弓箭、爆炸与持续伤害均实时在受击位置弹出清晰描边数字：
  [list]
  [*][color=#ffffff][b]白色[/b][/color] — 普通命中
  [*][color=#ffd23f][b]金色放大[/b][/color] — 重击/高伤（单次 ≥ 最大血量 35% 或 ≥ 400 伤害，可自定义）
  [*][color=#ff453a][b]鲜红放大 + "!"[/b][/color] — 击杀一击
  [/list]
[*][b]延迟扣血残影条 + 多目标追踪：[/b]绿/黄/红分段血条搭配暖色扣血残影，范围攻击与爆炸可同时显示多个受击敌人的血条与伤害数字。
[*][b]伴侣模式：[/b]如果你更喜欢保留原版 / crinloader / 7px 血条，按 [b]F6[/b]（或在 INI 中设 [font=Courier New]draw_custom_bar=0[/font]）即可切换为仅叠加血量数字与伤害跳字。
[/list]

[size=3][b]游戏内热键[/b][/size]
[list]
[*][b]F5[/b] — 实时开关整个覆盖层。
[*][b]F6[/b] — 循环切换模式：[b]自定义血条+数字[/b] → [b]仅血量数字（配合原版血条）[/b] → [b]仅显示伤害跳字[/b]。
[/list]

[size=3][b]安装与卸载[/b][/size]
[b]安装：[/b]解压后将 [font=Courier New]winmm.dll[/font] 和 [font=Courier New]dl1_hp_damage.ini[/font] 复制到游戏根目录（与 [font=Courier New]DyingLightGame.exe[/font] 同级）即可。
[b]卸载：[/b]删除游戏根目录下的 [font=Courier New]winmm.dll[/font]、[font=Courier New]dl1_hp_damage.ini[/font] 和 [font=Courier New]dl1_hp_damage.log[/font]。

[size=3][b]兼容性与联机提醒[/b][/size]
[list]
[*]零冲突独立运行：通过 [font=Courier New]winmm.dll[/font] 代理加载，不修改任何 [font=Courier New].pak[/font] 文件，完美兼容 [b]ReShade[/b]（[font=Courier New]dxgi.dll[/font]）、[b]crinloader[/b]（[font=Courier New]dsound.dll[/font]）、[b]DLMM[/b] 及全部武器/脚本模组。
[*][color=#ff6b6b][b]联机注意：[/b][/color]本模组涉及内存读取与 DX11 渲染注入，仅限单人游玩或关闭 VAC 的私人联机，请勿在开启 VAC 的公共服务器使用。
[/list]

[line]
[size=3][b]Open Source / 开源仓库 (MIT License)[/b][/size]
[url=https://github.com/Soul-Charge/dl1-hp-damage-numbers]GitHub Repository — Soul-Charge/dl1-hp-damage-numbers[/url]
```

## 4. Changelog (v1.0.1)
```text
v1.0.1
- Added built-in English / Simplified Chinese bilingual support (auto-detects Windows UI language, or force via `language=auto|en|zh` in dl1_hp_damage.ini).
- Added bilingual (EN/CN) comments in dl1_hp_damage.ini and README.txt.
- Fixed scaled MaxHP consistency on Nightmare/Legend difficulties.
```
