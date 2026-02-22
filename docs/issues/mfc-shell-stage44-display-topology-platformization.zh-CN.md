# MFC 壳解耦阶段 44：显示器拓扑能力平台化（替代 MonitorUtils）

## 1. 背景与目标

阶段 43 后，`MouseFx` 仍保留一块 Win32 绑定工具：

- `MouseFx/Utils/MonitorUtils.h/.cpp`

该模块直接暴露 `RECT/POINT`，并被：

- `MouseFx/Server/SettingsSchemaBuilder.cpp`（Core/Server）
- `Platform/windows/Overlay/Win32InputIndicatorOverlay.cpp`（平台实现）

共同依赖，导致 Core 层仍受 Win32 类型污染。

本阶段目标：

- 将显示器枚举/定位能力迁移到 `Platform` 门面 + `Platform/windows` 实现；
- 删除 `MouseFx/Utils/MonitorUtils.*`；
- Core/Server 侧仅依赖平台无关数据结构。

## 2. 判定

判定：`架构演进`（平台边界收口，不改变用户可见行为）。

依据：

- 监视器枚举、排序、主屏/光标屏解析逻辑沿用旧实现；
- 配置面板监视器下拉/多屏定位语义未改；
- 双配置构建通过，运行链路保持一致。

## 3. 实施内容

### 3.1 新增平台门面与 Win32 实现

新增文件：

- `MFCMouseEffect/Platform/PlatformDisplayTopology.h`
- `MFCMouseEffect/Platform/PlatformDisplayTopology.cpp`
- `MFCMouseEffect/Platform/windows/System/Win32DisplayTopology.h`
- `MFCMouseEffect/Platform/windows/System/Win32DisplayTopology.cpp`

新增平台无关类型：

- `platform::DisplayRect`
- `platform::DisplayPoint`
- `platform::DisplayMonitorEntry`

新增 API：

- `platform::EnumerateDisplayMonitors()`
- `platform::ResolveTargetDisplayMonitor(...)`
- `platform::ResolveTargetDisplayMonitorBounds(...)`

### 3.2 调整调用方

变更文件：

- `MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.cpp`
  - 改用 `Platform/PlatformDisplayTopology.h`
  - `mousefx::EnumMonitors()` -> `platform::EnumerateDisplayMonitors()`

- `MFCMouseEffect/Platform/windows/Overlay/Win32InputIndicatorOverlay.cpp`
  - 改用平台门面
  - 绝对定位与多屏 clone 定位均改为 `platform::ResolveTargetDisplayMonitor(...)`/`platform::EnumerateDisplayMonitors()`

### 3.3 删除旧模块

删除：

- `MFCMouseEffect/MouseFx/Utils/MonitorUtils.h`
- `MFCMouseEffect/MouseFx/Utils/MonitorUtils.cpp`

### 3.4 工程文件同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 新增 `PlatformDisplayTopology` 与 `Win32DisplayTopology` 源/头；
- 移除 `MonitorUtils` 源/头条目。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 量化结果

`MouseFx + UI` 范围：

- `#include <windows.h>`：`8 -> 7`

当前剩余主要集中在：

- GPU/渲染相关头
- `GdiPlusSession` 核心启动封装

## 6. 风险与后续

已控制：

- `SettingsSchemaBuilder` 仍获得完整监视器信息（id/name/bounds/is_primary）；
- `Win32InputIndicatorOverlay` 仍使用 Win32 窗口定位，但坐标来源已平台门面化。

后续建议：

- 继续处理 `MouseFx/Core/System/GdiPlusSession.h` 的 Win32 头暴露；
- 评估 GPU 渲染模块中可进一步抽离的平台接口（保持高性能路径不回退）。

