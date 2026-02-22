# MFC 壳解耦阶段 26：光标查询门面与 Layer/Renderer 去 Win32 直连

## 1. 背景与目标

阶段 24/25 已把坐标协议统一到 `ScreenPoint`，但仍有一类残留：

- `TrailOverlayLayer`、`ParticleTrailOverlayLayer`、`HoldQuantumHaloGpuV2Renderer` 直接调用 `GetCursorPos`；
- 这类调用绕过了已存在的平台抽象 `ICursorPositionService`，导致平台边界不统一。

本阶段目标：

- 新增 Core 侧统一光标查询门面；
- 消除 Layer/Renderer 里的 `GetCursorPos` 直连；
- 保持行为不变，仅收敛依赖路径。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 光标数据来源仍是平台实现 `Win32CursorPositionService`；
- 对外可见行为（鼠标轨迹/粒子/Hold）不应变化，主要是依赖方向收敛。

## 3. 实施内容

### 3.1 新增光标查询门面（Core）

新增：

- `MFCMouseEffect/MouseFx/Core/System/CursorPositionProvider.h`
- `MFCMouseEffect/MouseFx/Core/System/CursorPositionProvider.cpp`

实现要点：

- 在 Core 侧提供 `TryGetCursorScreenPoint`；
- 内部通过 `platform::CreateCursorPositionService()` 选择平台实现；
- 若平台服务不可用，自动回退 `NullCursorPositionService`。

### 3.2 Layer 路径去 Win32 直连

修改：

- `MFCMouseEffect/MouseFx/Layers/TrailOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Layers/ParticleTrailOverlayLayer.h`
- `MFCMouseEffect/MouseFx/Layers/ParticleTrailOverlayLayer.cpp`

关键点：

- `TrailOverlayLayer` 不再直接 `GetCursorPos`，改走门面；
- `ParticleTrailOverlayLayer`：
  - `UpdateCursor`、`Emit` 与内部缓存点统一为 `ScreenPoint`；
  - `GetCursorPos` 改为门面调用。

### 3.3 Effect/Renderer 同步

修改：

- `MFCMouseEffect/MouseFx/Effects/ParticleTrailEffect.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`

关键点：

- `ParticleTrailEffect` 在 host 模式直接传 `ScreenPoint` 给 layer；
- `HoldQuantumHaloGpuV2Renderer` 改用门面查询光标，不再直连 `GetCursorPos`。

### 3.4 工程文件同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增编译项：

- `MouseFx/Core/System/CursorPositionProvider.h/.cpp`

## 4. 验证

构建验证（VS 2026 MSBuild amd64）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

说明：

- 首次 `Release` 出现 `LNK1104`（目标 exe 被占用）；结束占用进程后复构建通过，属于环境占用问题。

## 5. 风险与边界

已控制：

- 运行时光标来源未变化（Win32 仍由 `Win32CursorPositionService` 提供）；
- Layer/Renderer 算法未变，仅替换取点入口。

当前边界：

- `MouseFx/Windows/TrailWindow.cpp`、`MouseFx/Windows/ParticleTrailWindow.cpp` 仍保留 Win32 直取点（属于 legacy fallback 窗口路径）。

## 6. 下一步建议（阶段 27）

- 清理 legacy fallback 窗口里的 `GetCursorPos` 直连并统一到门面；
- 评估将 `CursorPositionProvider` 注入化（替代静态单例），提升测试可控性。
