# DL1 Enemy Health Bar Numbers & Floating Damage Numbers

《消逝的光芒 1》(Dying Light 1.55, Steam x64) 原生 DX11 覆盖层模组：
- **敌人血条 + 精确血量数字**（如 `420 / 500`，可选百分比 `420 / 500 (84%)`），支持自定义现代 RPG 血条（绿/黄/红分段 + 延迟扣血残影条 + 距离缩放与淡出），同时支持 **纯数字伴侣模式**（保留原版 / crinloader / `HealthBars7px.pak` 血条，仅叠加血量数字）。
- **攻击实时飘出伤害数字**（支持近战、枪械、弓箭、投掷物、爆炸、AOE 多目标与持续伤害）：
  - 普通伤害：白色描边数字；
  - 重击 / 高伤（单次 ≥ 目标最大生命 35% 或 ≥ 400）：金色放大数字；
  - 击杀一击：鲜红放大数字并附带 `!`。
- **零冲突**：通过 `winmm.dll` 代理加载，不占用 `dsound.dll`（crinloader）与 `dxgi.dll`（ReShade），不修改任何 `.pak` / `.rpack` 文件，兼容所有武器/脚本/画质模组。

## 安装与卸载

### 安装
将 `dist/` 目录下的两个文件复制到《消逝的光芒 1》游戏根目录（与 `DyingLightGame.exe` 同级）：
- `dist/winmm.dll`
- `dist/dl1_hp_damage.ini`

> 注意：本模组修改内存并注入 DX11 渲染，请仅在单人或关闭 VAC 的私人联机中使用（不要在开启 VAC 的公共房间使用）。

### 卸载
删除游戏根目录下的 `winmm.dll`、`dl1_hp_damage.ini` 和 `dl1_hp_damage.log` 即可完全还原。

## 游戏内热键

- **`F9`**：实时开关整个血条与伤害数字覆盖层（左上角短暂提示状态）。
- **`F10`**：实时循环切换血条模式：
  1. `Custom Health Bar + HP Numbers`（自定义血条 + 精确血量数字，自动隐藏原版血条避免重叠）
  2. `HP Numbers Only (Vanilla Bar)`（仅显示精确血量数字，配合 crinloader / 7px 等原版血条使用）
  3. `Damage Numbers Only`（仅显示攻击伤害数字）

所有开关、字号、距离、持续时间、暴击阈值均可在 `dl1_hp_damage.ini` 中自定义。

## 从源码构建

需要 Visual Studio 2022（含 C++ x64 工具链与 Windows SDK）：

```powershell
# 仅构建到 dist/winmm.dll
pwsh -NoProfile -File build.ps1

# 构建并安装到游戏目录
pwsh -NoProfile -File build.ps1 -Install
```

## 技术原理

1. **`winmm.dll` 代理**：动态从 `GetSystemDirectoryW()\winmm.dll` 解析全部 180 个系统导出，通过 MASM x64 跳板（`src/winmm_stubs.asm`）零开销转发。
2. **游戏数据 Hook（`src/game_hooks.cpp`）**：
   - Hook `gamedll_x64_rwdi.dll` 的 `CHudEnemyHpItem::Update`（RVA `0x5d0740`），获取准星瞄准敌人的指针、精确当前血量与最大血量；若未安装 crinloader，则自动在 `0x5d0d91` 开启 `HudEnemyHp` 层。
   - Hook `engine_x64_rwdi.dll` 导出的 `IControlObject::TakeDamage`（动态 `GetProcAddress` 解析），通过 `IsAI()` 与 `HealthModule::owner == enemy` 双重校验 + SEH 保护，读取受击前后 `HealthModule + 0x78` 的精确血量差值，生成 3D 锚点伤害飘字并追踪所有近期受击敌人。
   - 通过 `IBaseCamera::GetCombinedMatrix`（动态导出解析）将敌人头部/受击 3D 坐标实时投影到屏幕像素。
3. **纯 D3D11 2D 渲染器（`src/d3d11_renderer.cpp`）**：
   - 不依赖 `D3D11_CREATE_DEVICE_BGRA_SUPPORT`（DL1 默认 `Flags=0`），使用 GDI 光栅化 `Segoe UI Bold` 字体图集 + 单次批处理 D3D11 着色器渲染，完整备份与还原 D3D11 管线状态，兼容 ReShade 6.x 与窗口大小变化。
