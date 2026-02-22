# MFC 壳解耦阶段 34：Win32 入口壳迁移到 Platform 目录

## 1. 背景与目标

阶段 33 后，Windows 托盘实现已收口到 `Platform/windows`，但主入口壳仍残留在：

- `MFCMouseEffect/MouseFx/Core/Shell/Win32AppShell.h/.cpp`

这会导致目录语义不一致：Windows 专属入口代码仍在 Core 下。

本阶段目标：

- 将 `Win32AppShell` 迁移到 `Platform/windows/Shell`；
- 同步入口引用和工程编译清单；
- 保持启动流程与行为不变。

## 2. 判定

判定：`架构演进`（目录收口，不改业务行为）。

依据：

- `Win32AppShell` 的命令行解析和 `AppShellCore` 启停流程未改；
- 仅迁移路径并更新 include/工程引用。

## 3. 实施内容

### 3.1 文件迁移

从：

- `MFCMouseEffect/MouseFx/Core/Shell/Win32AppShell.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/Shell/Win32AppShell.h/.cpp`

### 3.2 入口引用同步

文件：

- `MFCMouseEffect/MFCMouseEffect.cpp`
- `MFCMouseEffect/MFCMouseEffect.h`
- `MFCMouseEffect/Platform/windows/Shell/Win32AppShell.cpp`

变更：

- include 路径改为 `Platform/windows/Shell/Win32AppShell.h`；
- 入口注释同步更新到新路径。

### 3.3 工程文件同步

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- `Win32AppShell` 头/源文件路径改为 `Platform/windows/Shell/*`；
- filters 映射到 `头文件\Platform` 与 `源文件\Platform`。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与边界

已控制：

- 保持 `AppShellCore` 接口与生命周期调用不变；
- 使用 `git mv` 保留历史，便于后续追踪。

当前剩余：

- `MouseFx/Core/Protocol/InputTypesWin32.h` 仍为 Windows 类型适配残留，后续可继续平台化收口。
