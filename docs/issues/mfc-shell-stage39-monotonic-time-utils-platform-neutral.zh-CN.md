# MFC 壳解耦阶段 39：单调时钟工具平台无关化

## 1. 背景与目标

在阶段 38 后，`MouseFx` 内仍存在多处直接 `GetTickCount64` 调用，主要分布在轨迹/悬浮渲染与 Hold 运行时逻辑中。  
这会让非平台层直接依赖 Win32 API，不利于后续 macOS/Linux 实现对齐。

本阶段目标：

- 将 `MouseFx` 层的时间读取统一到 `NowMs()`；
- 将 `NowMs()` 实现改为 `std::chrono::steady_clock`；
- 把 `GetTickCount64` 依赖收敛到 `Platform/windows`。

## 2. 判定

判定：`架构演进`（实现方式替换，不改功能行为）。

依据：

- `NowMs()` 仍提供单调毫秒时间戳；
- 各效果逻辑仍以“当前毫秒 - 轨迹时间/起始时间”计算寿命和淡出；
- 构建验证通过，行为路径未变更。

## 3. 实施内容

### 3.1 TimeUtils 改为标准库单调时钟

文件：

- `MFCMouseEffect/MouseFx/Utils/TimeUtils.h`

变更：

- 移除 `windows.h` 依赖；
- `NowMs()` 改为 `std::chrono::steady_clock` + `duration_cast<milliseconds>`。

### 3.2 渲染与运行时统一改用 NowMs

文件：

- `MFCMouseEffect/MouseFx/Renderers/Trail/TubesRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/StreamerTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/MeteorRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/LineTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/ElectricTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Hover/TubesHoverRenderer.h`
- `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.cpp`

变更：

- 以上文件所有 `GetTickCount64()` 替换为 `NowMs()`；
- 补齐 `MouseFx/Utils/TimeUtils.h` 头文件依赖；
- 保持原有计算参数和时序阈值不变。

### 3.3 结果收敛

通过全局搜索确认：

- `MouseFx` 目录内 `GetTickCount64` 已清零；
- 剩余调用仅位于 `Platform/windows`（符合平台边界）。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与后续

已控制：

- 使用 `steady_clock` 保证单调性，避免系统时间回拨影响；
- 效果时间计算保持毫秒粒度与历史逻辑一致。

后续建议：

- 继续梳理 `MouseFx` 中其余 Win32 API 依赖（如编码转换、资源访问）；
- 将可抽象能力继续沉淀到 `Platform/*` + 统一门面，减少核心层平台分支。
