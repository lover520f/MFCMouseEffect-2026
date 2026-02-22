# MFC 壳解耦阶段 28：文本窗口链路 `ScreenPoint` 收敛

## 1. 背景与目标

当前文本特效链路里，`TextOverlayLayer` 已经使用 `ScreenPoint`，但 `TextWindowPool/TextWindow` 仍暴露 `POINT`，导致：

- `TextEffect` 和 `OverlayHostService` 需要做 `ScreenPoint -> POINT` 转换；
- 核心层继续依赖 `InputTypesWin32.h`，平台类型外泄。

本阶段目标：

- 将文本窗口接口统一为 `ScreenPoint`；
- 清理文本链路里不必要的 Win32 转换与头文件依赖。

## 2. 判定

判定：`架构演进`（接口边界收敛，非功能回归修复）。

依据：

- 用户可见行为不变（文本特效展示逻辑不变）；
- 变更点集中在类型边界与调用链整理。

## 3. 实施内容

### 3.1 TextWindow 接口收敛

文件：

- `MFCMouseEffect/MouseFx/Windows/TextWindow.h`
- `MFCMouseEffect/MouseFx/Windows/TextWindow.cpp`

变更：

- `StartAt(const POINT& ...)` 改为 `StartAt(const ScreenPoint& ...)`；
- 删除未使用的 `startPt_` 成员；
- `TextWindow` 直接使用 `ScreenPoint`，仅在 Win32 API 调用处使用 `x/y` 基础值。

### 3.2 TextWindowPool 接口收敛

文件：

- `MFCMouseEffect/MouseFx/Windows/TextWindowPool.h`
- `MFCMouseEffect/MouseFx/Windows/TextWindowPool.cpp`

变更：

- `ShowText(const POINT& ...)` 改为 `ShowText(const ScreenPoint& ...)`；
- 池内转发调用 `TextWindow::StartAt` 时保持 `ScreenPoint` 传递。

### 3.3 文本特效调用链去 Win32 转换

文件：

- `MFCMouseEffect/MouseFx/Effects/TextEffect.cpp`
- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.cpp`

变更：

- 删除 `InputTypesWin32.h` 依赖；
- 删除 `ToNativePoint(...)` 转换；
- 直接传递 `ScreenPoint` 给 `TextWindowPool`。

## 4. 验证

使用 VS 2026 MSBuild（x64 工具链）构建：

- `Debug|x64`：通过
- `Release|x64`：通过

命令：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m /nologo`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m /nologo`

## 5. 风险与边界

已控制：

- 仅调整文本窗口链路的类型边界，不改动画参数、渲染路径与生命周期；
- 编译回归覆盖 Debug/Release 双配置。

边界：

- `TextWindow` 仍是 Win32 实现（窗口创建、定时器、分层窗体），本阶段仅做类型面收敛；
- 后续若继续平台化，应把 `TextWindow` 放入平台渲染后端能力下沉。
