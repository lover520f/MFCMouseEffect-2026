# MFC 壳解耦阶段 30：Effect 后备窗口接口化与平台工厂注入

## 1. 背景与目标

在阶段 28/29 后，`TrailEffect`、`ParticleTrailEffect`、`TextEffect` 仍直接依赖：

- `MouseFx/Windows/TrailWindow.h`
- `MouseFx/Windows/ParticleTrailWindow.h`
- `MouseFx/Windows/TextWindowPool.h`

这会让效果层直接感知 Win32 后备实现，不利于后续将实现下沉到 `Platform/windows` 并为 macOS/Linux 预留替换点。

本阶段目标：

- 为三类后备能力建立稳定接口；
- 使用平台工厂注入具体实现；
- 将 Effect 头文件从 `MouseFx/Windows/*` 依赖中解耦。

## 2. 判定

判定：`架构演进`（边界收敛，不改变功能行为）。

依据：

- 对外特效行为不变（Overlay 成功仍优先 Overlay，后备逻辑仍可用）；
- 仅调整依赖方向与实例化方式。

## 3. 实施内容

### 3.1 新增后备接口与空实现

目录：`MFCMouseEffect/MouseFx/Interfaces/`

新增：

- `ITextEffectFallback.h`
- `ITrailEffectFallback.h`
- `IParticleTrailEffectFallback.h`
- `NullTextEffectFallback.h`
- `NullTrailEffectFallback.h`
- `NullParticleTrailEffectFallback.h`

作用：

- 统一定义 Effect 需要的最小后备能力；
- 非 Windows 平台提供空实现，保证编译与架构边界稳定。

### 3.2 新增平台工厂

文件：

- `MFCMouseEffect/Platform/PlatformEffectFallbackFactory.h`
- `MFCMouseEffect/Platform/PlatformEffectFallbackFactory.cpp`

提供：

- `CreateTextEffectFallback()`
- `CreateTrailEffectFallback()`
- `CreateParticleTrailEffectFallback()`

策略：

- Windows 返回 Win32 适配器实现；
- 其他平台返回 Null 实现。

### 3.3 新增 Win32 适配器（包裹旧实现）

目录：`MFCMouseEffect/Platform/windows/Effects/`

新增：

- `Win32TextEffectFallback.h/.cpp`（封装 `TextWindowPool`）
- `Win32TrailEffectFallback.h/.cpp`（封装 `TrailWindow`）
- `Win32ParticleTrailEffectFallback.h/.cpp`（封装 `ParticleTrailWindow`）

说明：

- 现阶段是“适配包裹”，不改渲染实现；
- 为下一阶段把 `MouseFx/Windows/*` 迁移目录做准备。

### 3.4 Effect 侧依赖反转

文件：

- `MFCMouseEffect/MouseFx/Effects/TextEffect.h/.cpp`
- `MFCMouseEffect/MouseFx/Effects/TrailEffect.h/.cpp`
- `MFCMouseEffect/MouseFx/Effects/ParticleTrailEffect.h/.cpp`

变更：

- 移除 `MouseFx/Windows/*` 头文件依赖；
- 改为持有对应 fallback 接口指针；
- 通过 `PlatformEffectFallbackFactory` 创建后备实例。

### 3.5 工程文件更新

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`

变更：

- 新增上述接口、工厂、Win32 适配器的头/源文件到工程编译清单。

## 4. 验证

构建命令（VS 2026 x64 工具链）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m /nologo`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m /nologo`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过（首次因 `MFCMouseEffect.exe` 占用导致 `LNK1104`，自动结束进程后重试通过）

## 5. 风险与边界

已控制：

- 行为路径不变，仅抽象层次变化；
- 适配器直接包裹旧实现，回归风险低。

当前边界：

- `MouseFx/Windows/*` 仍然存在且被 Win32 适配器调用；
- 下一阶段应迁移这些具体实现到 `Platform/windows/*`（或等价目录）完成物理收口。
