# MFC 壳解耦阶段 41：虚拟键协议常量抽象与 Win32 头依赖缩减

## 1. 背景与目标

阶段 40 后，自动化快捷键链路中仍存在直接 `VK_*` 宏依赖：

- `MouseFx/Core/Automation/KeyChord.h/.cpp`
- `MouseFx/Core/Automation/ShortcutCaptureSession.cpp`

这使核心逻辑继续依赖 `<windows.h>`。同时 `TrailColor.h` 也仅因 `BYTE` 依赖 Win32 头。

本阶段目标：

- 将虚拟键值迁移到协议常量头；
- 移除快捷键解析/捕获代码中的直接 Win32 头依赖；
- 清理 `TrailColor.h` 的 Win32 头依赖。

## 2. 判定

判定：`架构演进`（协议常量替代平台宏，不改行为）。

依据：

- 虚拟键数值与 Win32 `VK_*` 常量保持一致；
- 快捷键解析和捕获输出文本规则不变；
- 构建验证通过，运行路径保持一致。

## 3. 实施内容

### 3.1 新增虚拟键协议常量头

新增：

- `MFCMouseEffect/MouseFx/Core/Protocol/VirtualKeyCodes.h`

提供：

- `mousefx::vk::*` 常量（Ctrl/Shift/Alt/Win、功能键、方向键、编辑键、NumPad 等）。

目的：

- 自动化核心逻辑直接引用协议常量，而非 Win32 宏。

### 3.2 KeyChord 去 Win32 头依赖

文件：

- `MFCMouseEffect/MouseFx/Core/Automation/KeyChord.h`
- `MFCMouseEffect/MouseFx/Core/Automation/KeyChord.cpp`

变更：

- `KeyChord` 内部键值类型从 `UINT` 改为 `uint32_t`；
- 解析逻辑改为使用 `mousefx::vk::*` 常量；
- 头文件移除 `<windows.h>` 依赖。

### 3.3 ShortcutCaptureSession 去显式 Win32 头依赖

文件：

- `MFCMouseEffect/MouseFx/Core/Automation/ShortcutCaptureSession.cpp`

变更：

- 删除 `#include <windows.h>`；
- 修饰键识别和按键 token 映射改为 `mousefx::vk::*` 常量。

### 3.4 TrailColor 头文件瘦身

文件：

- `MFCMouseEffect/MouseFx/Utils/TrailColor.h`

变更：

- 移除 `<windows.h>`；
- `BYTE` 改为 `uint8_t` 并使用显式 `static_cast<uint8_t>`。

### 3.5 工程文件同步

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 新增 `MouseFx/Core/Protocol/VirtualKeyCodes.h` 项目与 filters 映射。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 量化结果

`MouseFx` + `UI` 范围内 `#include <windows.h>` 数量由 13 处降到 10 处（本阶段减少 3 处）。

## 6. 风险与后续

已控制：

- 虚拟键值采用 Win32 标准值，快捷键行为不变；
- `KeyboardInjector` 仍位于平台注入路径，键值类型转换保留。

后续建议：

- 继续处理 `WebUiAssets.cpp`、`StringUtils.cpp`、`MonitorUtils.h` 等剩余 Win32 绑定点；
- 将 GPU/渲染相关 Win32 头依赖按模块逐步迁入 `Platform/windows`。
