# MFC 壳解耦阶段 35：Win32 输入类型适配头收口到 Platform

## 1. 背景与目标

阶段 34 后，`MouseFx/Core` 目录中仍有一个 Windows 专属适配头：

- `MFCMouseEffect/MouseFx/Core/Protocol/InputTypesWin32.h`

该文件只做 `POINT <-> ScreenPoint` 转换，属于平台适配实现，不应放在 Core。

本阶段目标：

- 将 Win32 输入点位适配头迁移到 `Platform/windows`；
- 清理 Core Effect 层中无效的 Win32 include；
- 保持输入/渲染行为完全不变。

## 2. 判定

判定：`架构演进`（边界收口，行为不变）。

依据：

- 迁移前后仅 include 路径改变，转换函数实现不变；
- `HoverEffect`、`HoldEffect` 中对旧头文件是未使用依赖，删除后不影响逻辑。

## 3. 实施内容

### 3.1 文件迁移

从：

- `MFCMouseEffect/MouseFx/Core/Protocol/InputTypesWin32.h`

迁移到：

- `MFCMouseEffect/Platform/windows/Protocol/Win32InputTypes.h`

说明：

- 保留函数名与签名：`ToScreenPoint`、`ToNativePoint`；
- 命名空间与实现不变，仅目录收口。

### 3.2 引用路径更新

更新文件：

- `MFCMouseEffect/Platform/windows/System/Win32GlobalMouseHook.cpp`
- `MFCMouseEffect/Platform/windows/Overlay/Win32InputIndicatorOverlay.cpp`

变更：

- include 从 `MouseFx/Core/Protocol/InputTypesWin32.h` 改为 `Platform/windows/Protocol/Win32InputTypes.h`。

### 3.3 Core 层无效依赖清理

更新文件：

- `MFCMouseEffect/MouseFx/Effects/HoverEffect.cpp`
- `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`

变更：

- 删除未使用的 `InputTypesWin32` include，减少 Core 对 Win32 适配文件的隐式耦合。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过（期间出现一次 `LNK1104` 因 `MFCMouseEffect.exe` 占用，自动结束占用进程后重试通过）

## 5. 风险与边界

已控制：

- 仅迁移头文件和 include 路径，不改事件分发与渲染代码；
- 使用 `git mv` 保留文件历史。

当前剩余（Windows 跨平台收口）：

- 仍有部分 `windows.h` 依赖位于非 `Platform/windows` 目录（如工具层、Wasm bridge、部分渲染头），后续阶段继续分批收口。
