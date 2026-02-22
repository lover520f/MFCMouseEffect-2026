# MFC 壳解耦阶段 31：OverlayHost 文本后备路径平台化

## 1. 背景与目标

阶段 30 后，`TextEffect` 已使用平台 fallback 接口，但 `OverlayHostService` 仍直接依赖：

- `MouseFx/Windows/TextWindowPool.h`

这会让核心 Overlay 服务继续感知 Win32 具体实现。

本阶段目标：

- 将 `OverlayHostService` 的 emoji 文本后备路径改为平台 fallback 接口；
- 去除 Core 层对 `TextWindowPool` 的直接引用。

## 2. 判定

判定：`架构演进`（边界收敛，行为不变）。

依据：

- emoji 文本仍走后备路径；
- 改动仅是实例化入口与依赖方向调整。

## 3. 实施内容

### 3.1 OverlayHostService 引入接口依赖

文件：

- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.h`

变更：

- 新增 `ITextEffectFallback` 前置声明；
- 新增成员 `std::unique_ptr<ITextEffectFallback> textFallback_`。

### 3.2 删除 TextWindowPool 静态共享池

文件：

- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.cpp`

变更：

- 删除 `#include "MouseFx/Windows/TextWindowPool.h"`；
- 删除 `SharedTextWindowPool()` 静态函数。

### 3.3 走平台工厂创建文本后备

文件：

- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.cpp`

变更：

- 新增 `#include "Platform/PlatformEffectFallbackFactory.h"`；
- `ShowText(...)` 在 emoji 路径改为：
  - 惰性创建 `textFallback_`；
  - `EnsureInitialized(8)`；
  - `ShowText(...)`。
- `Shutdown()` 时调用 `textFallback_->Shutdown()`。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m /nologo`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m /nologo`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与边界

已控制：

- fallback 接口与 Win32 适配器由阶段 30 提供，调用语义一致；
- 不改变 Overlay/Layer 逻辑，仅替换文本后备实例来源。

当前边界：

- `MouseFx/Windows/TextWindowPool` 仍由 Win32 适配器使用；
- 下一步应把 `MouseFx/Windows/*` 的物理实现迁移到 `Platform/windows/*`，彻底完成目录层收口。
