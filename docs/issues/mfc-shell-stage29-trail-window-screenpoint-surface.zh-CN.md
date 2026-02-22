# MFC 壳解耦阶段 29：Trail 窗口链路 `ScreenPoint` 收敛

## 1. 背景与目标

在阶段 28 完成文本窗口链路后，`TrailEffect/ParticleTrailEffect` 仍保留：

- `ScreenPoint -> POINT` 的宿主转换；
- 对 `InputTypesWin32.h` 的直接依赖。

本阶段目标：

- 将 `TrailWindow` 与 `ParticleTrailWindow` 的输入接口统一到 `ScreenPoint`；
- 继续压缩效果层到 Win32 类型的耦合面。

## 2. 判定

判定：`架构演进`（类型边界统一，非行为回归修复）。

依据：

- 对外交互与特效表现不变；
- 变更仅在输入坐标类型与调用链依赖。

## 3. 实施内容

### 3.1 TrailWindow 接口收敛

文件：

- `MFCMouseEffect/MouseFx/Windows/TrailWindow.h`
- `MFCMouseEffect/MouseFx/Windows/TrailWindow.cpp`

变更：

- `AddPoint(const POINT&)` 改为 `AddPoint(const ScreenPoint&)`；
- `latestCursorPt_ / lastSamplePt_` 改为 `ScreenPoint`；
- 采样逻辑中直接复用 `TryGetCursorScreenPoint` 输出，移除中间 `POINT` 转换。

### 3.2 ParticleTrailWindow 接口收敛

文件：

- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.h`
- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.cpp`

变更：

- `UpdateCursor(const POINT&)` 改为 `UpdateCursor(const ScreenPoint&)`；
- `Emit(const POINT&, ...)` 改为 `Emit(const ScreenPoint&, ...)`；
- `latestCursorPt_ / lastEmitCursorPt_` 改为 `ScreenPoint`；
- Tick 采样路径去除 `POINT` 中转。

### 3.3 效果层去 Win32 转换

文件：

- `MFCMouseEffect/MouseFx/Effects/TrailEffect.cpp`
- `MFCMouseEffect/MouseFx/Effects/ParticleTrailEffect.cpp`

变更：

- 删除 `InputTypesWin32.h` 依赖；
- 删除 `ToNativePoint(pt)` 调用，直接传递 `ScreenPoint` 给窗口后备路径。

## 4. 验证

使用 VS 2026 x64 MSBuild 构建：

- `Debug|x64`：通过
- `Release|x64`：通过

命令：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m /nologo`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m /nologo`

## 5. 风险与边界

已控制：

- 仅改输入坐标类型，不改渲染参数、绘制策略和生命周期；
- 保留现有 Win32 窗口实现，确保稳定性与回归风险可控。

边界：

- 当前只是“类型面收敛”，尚未把 Trail 窗口后备实现下沉到完整平台后端抽象；
- 后续可继续按同模式把 legacy 窗口后备路径逐步收口到 `Platform/windows`。
