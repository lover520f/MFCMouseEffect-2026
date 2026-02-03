# 拖尾特效：差异化与“历史长度”修复

## 总结
“绚丽流星（meteor） / 霓虹流光（streamer） / 赛博电弧（electric）”之前看起来很像，核心原因是它们都被同一个 `TrailWindow` 的历史窗口（固定的 `durationMs_` 与 `maxPoints_`）限制了：  
哪怕 renderer 里写了更长/更短的时长，点位早就被窗口裁掉，最终拖尾长度与密度趋同。

本次改动：
- 给每种拖尾类型增加**独立的历史配置**（点位保留时长 + 点位数量）。
- 把旧的“一个大头文件里塞 3 个 renderer”拆成更小、更单一职责的文件。
- 拖尾渲染去掉全局 `rand()/srand()`（避免污染全局随机状态）。
- 对三个 renderer 做了风格调参，确保“辨识度”明显不同。

## 根因
`TrailWindow` 每帧会根据内部 `durationMs_` / `maxPoints_` 删除旧点。
- 之前这些参数对所有拖尾类型基本是统一的。
- renderer 内部的 `durationMs` 即使写了，也往往“用不上”，因为点已经被窗口删掉了。

结果：拖尾“长度/密度/采样”一致，视觉自然就很像。

## 实现要点

### 1）每个类型独立的历史配置
配置入口：
- `MFCMouseEffect/MouseFx/Core/EffectFactory.cpp`（注入 profile）
- `MFCMouseEffect/MouseFx/Effects/TrailEffect.cpp`（应用到 `TrailWindow`）

当前配置：
- `electric`：`durationMs=280`，`maxPoints=24`
- `streamer`：`durationMs=420`，`maxPoints=46`
- `meteor`：`durationMs=520`，`maxPoints=60`
- `tubes/scifi`：`durationMs=350`，`maxPoints=40`
- 默认 `line`：`durationMs=300`，`maxPoints=32`

覆盖方式：
- `config.json` 根节点 `trail_profiles`（详见 `docs/architecture/trail-profiles-config.zh-CN.md`）

### 2）TrailWindow 支持可配置
`TrailWindow` 新增：
- `SetDurationMs(int)`
- `SetMaxPoints(int)`

让 renderer 真正拿到足够的历史点位，才能做出“不同拖尾长度/密度”的效果。

### 3）Renderer 拆分（单一职责）
旧文件：`MouseFx/Interfaces/TrailRenderStrategies.h`

现在该文件仅作为聚合 include，真实实现拆分到：
- `MouseFx/Renderers/Trail/LineTrailRenderer.h`
- `MouseFx/Renderers/Trail/StreamerTrailRenderer.h`
- `MouseFx/Renderers/Trail/ElectricTrailRenderer.h`

### 4）三个特效的“辨识度”调参（概要）
- **霓虹流光**：双通道描边（外层 glow + 内层 core），头部更粗更亮，像“霓虹丝带”。
- **赛博电弧**：稳定的电弧抖动（按帧桶 seed），白色核心 + 偶发分叉，更像“电弧”而不是“抖动线条”。
- **绚丽流星**：非 chromatic 时改为偏暖尾焰，火花更明显，头部增加方向性光晕/闪烁。

### 5）复用工具（减少重复）
- `MouseFx/Utils/TrailColor.h`（HSL → `Gdiplus::Color`）
- `MouseFx/Utils/TrailMath.h`（Clamp/Idle 淡出因子）
- `MouseFx/Utils/XorShift.h`（轻量 PRNG + seed mix）

## 手动验证
1. 运行 `x64\\Debug\\MFCMouseEffect.exe`。
2. 设置 → 拖尾：分别切到 `meteor / streamer / electric`。
3. 鼠标画圈移动并突然停下：
   - `electric` 应该更“短促”，并出现电弧分叉。
   - `streamer` 应该是更平滑的霓虹丝带（外层发光明显）。
   - `meteor` 应该有明显火花粒子与偏暖尾焰。

热应用：
- 修改 `config.json` 的 `trail_profiles` 后，通过托盘菜单 `重载配置 (Reload config)` 或 IPC `{"cmd":"reload_config"}` 应用。
