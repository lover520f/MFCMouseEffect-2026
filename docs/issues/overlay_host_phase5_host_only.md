# OverlayHost Phase 5: Ripple Family Host-only 收敛

## 1. 目标

把 ripple 家族效果彻底收敛到 `OverlayHostWindow`，避免再次回退到 `RippleWindowPool` 产生多画板并发。

覆盖范围：

- click: `RippleEffect`
- click: `IconEffect` (star)
- scroll: `ScrollEffect`
- hold: `HoldEffect`
- hover: `HoverEffect`

## 2. 本次改动

### 2.1 移除旧窗口池依赖

以下效果类不再持有 `RippleWindowPool`，并删除对应 `Initialize/Shutdown` 池管理逻辑：

- `MouseFx/Effects/RippleEffect.h/.cpp`
- `MouseFx/Effects/IconEffect.h/.cpp`
- `MouseFx/Effects/ScrollEffect.h/.cpp`
- `MouseFx/Effects/HoldEffect.h/.cpp`
- `MouseFx/Effects/HoverEffect.h/.cpp`

### 2.2 统一 Host 渲染入口

上述效果统一通过 `OverlayHostService` 进入 `RippleOverlayLayer`：

- one-shot: `ShowRipple(...)`
- continuous: `ShowContinuousRipple(...)`
- session 控制: `UpdateRipplePosition(...)` / `StopRipple(...)` / `SendRippleCommand(...)`

### 2.3 显式样式依赖

因为移除了 `RippleWindowPool` 的传递包含，效果头文件增加 `RippleStyle.h` 显式包含，避免编译期类型丢失。

## 3. 行为边界

- 当前仍保留 text 的 emoji 回退路径：`TextEffect -> TextWindowPool`（仅 emoji 场景）
- ripple 家族不再在 Host 失败时自动回退到旧 `RippleWindowPool`

## 4. 验证

已完成：

- `MSBuild /t:ClCompile` (`Release|x64`) 通过

说明：

- 完整链接 `MFCMouseEffect.exe` 在本地会受“目标程序正在运行”影响（`LNK1104` 文件占用），属于运行态限制，不是本次代码错误。
