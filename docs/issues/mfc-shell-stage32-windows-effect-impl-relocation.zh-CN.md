# MFC 壳解耦阶段 32：Windows 特效后备实现目录收口

## 1. 背景与目标

阶段 30/31 后，效果层与 Core 已通过平台 fallback 接口解耦，但具体实现文件仍位于：

- `MFCMouseEffect/MouseFx/Windows/*`

这会造成目录语义不一致：Windows 平台实现未完全落到 `Platform/windows`。

本阶段目标：

- 将这批 Win32 特效后备实现物理迁移到 `Platform/windows/Effects`；
- 保持行为与类名不变，仅收敛目录边界。

## 2. 判定

判定：`架构演进`（目录与边界治理，不改功能行为）。

依据：

- 迁移前后调用路径一致，仍由 Win32 fallback 适配器驱动；
- 主要改动是文件位置与工程引用。

## 3. 实施内容

### 3.1 文件迁移（git rename）

从：

- `MFCMouseEffect/MouseFx/Windows/TextWindow.h/.cpp`
- `MFCMouseEffect/MouseFx/Windows/TextWindowPool.h/.cpp`
- `MFCMouseEffect/MouseFx/Windows/TrailWindow.h/.cpp`
- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/Effects/TextWindow.h/.cpp`
- `MFCMouseEffect/Platform/windows/Effects/TextWindowPool.h/.cpp`
- `MFCMouseEffect/Platform/windows/Effects/TrailWindow.h/.cpp`
- `MFCMouseEffect/Platform/windows/Effects/ParticleTrailWindow.h/.cpp`

### 3.2 适配器头文件更新

文件：

- `MFCMouseEffect/Platform/windows/Effects/Win32TextEffectFallback.h`
- `MFCMouseEffect/Platform/windows/Effects/Win32TrailEffectFallback.h`
- `MFCMouseEffect/Platform/windows/Effects/Win32ParticleTrailEffectFallback.h`

变更：

- include 路径从 `MouseFx/Windows/*` 改为 `Platform/windows/Effects/*`。

### 3.3 工程文件更新

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`

变更：

- 头文件与编译单元引用路径同步迁移到 `Platform/windows/Effects/*`；
- 移除 `MouseFx/Windows/*` 的工程引用。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m /nologo`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m /nologo`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与边界

已控制：

- 使用 `git mv` 保留文件历史；
- 仅迁移路径，不改实现逻辑。

当前边界：

- `UI/Tray` 与少量 Win32 基础设施仍在平台目录外；
- 下一阶段应处理托盘与入口层收口，完成 Windows 迁移尾项。
