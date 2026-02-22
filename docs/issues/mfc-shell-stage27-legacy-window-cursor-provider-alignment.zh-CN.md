# MFC 壳解耦阶段 27：legacy 窗口光标查询路径对齐

## 1. 背景与目标

阶段 26 已清理 Core Layer/Renderer 的 `GetCursorPos` 直连，但 legacy fallback 窗口仍残留直连：

- `TrailWindow`
- `ParticleTrailWindow`

这会让同一功能在 host/fallback 两条路径上走不同光标来源链路。

本阶段目标：

- 将 legacy 窗口的光标查询统一切到 `CursorPositionProvider`；
- 保持窗口渲染逻辑和行为不变。

## 2. 判定

判定：`架构演进`（非功能回归修复）。

依据：

- 仅替换取点入口，不调整粒子/轨迹算法与窗口渲染流程。

## 3. 实施内容

修改：

- `MFCMouseEffect/MouseFx/Windows/TrailWindow.cpp`
- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.cpp`

关键调整：

- 引入 `MouseFx/Core/System/CursorPositionProvider.h`；
- `GetCursorPos` 调用改为 `TryGetCursorScreenPoint`；
- 返回值在窗口内按原数据结构转换回 `POINT`，确保 legacy 代码路径最小扰动。

## 4. 验证

构建验证（VS 2026 MSBuild amd64）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

附加核查：

- 全仓搜索 `GetCursorPos` 后，仅剩 `Platform/windows` 目录中的平台实现与 Win32 hook 代码。

## 5. 风险与边界

已控制：

- legacy 窗口输入数据与渲染坐标系未改；
- 仅替换光标读取来源，回归风险较低。

当前边界：

- 平台目录内仍保留 `GetCursorPos`（这是预期：平台实现应负责系统 API 调用）。

## 6. 下一步建议（阶段 28）

- 评估把 `CursorPositionProvider` 从静态门面演进为可注入依赖；
- 为 fallback 窗口路径补充最小行为回归清单（轨迹、粒子、长按联动）。
