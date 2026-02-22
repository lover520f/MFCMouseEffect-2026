# MFC 壳解耦阶段 13：Overlay 坐标空间服务平台化（Win32 首落地）

## 1. 背景与目标

阶段 12 完成了 Overlay Host Window 物理迁移，但 `OverlayCoordSpace.cpp` 仍直接使用 Win32 全局状态与 `GetSystemMetrics`，Core 层继续承载平台细节。

本阶段目标：

- 为 Overlay 坐标空间引入 Core 抽象接口；
- 在 `Platform/windows` 落地 Win32 实现；
- 保留现有对外 API（`OverlayCoordSpace.h`）以控制改动面。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 用户侧渲染行为不变；
- 主要变化为坐标服务职责下沉与依赖方向调整。

## 3. 实施内容

### 3.1 Core 新增坐标空间抽象与空实现

新增：

- `MFCMouseEffect/MouseFx/Core/Overlay/IOverlayCoordSpaceService.h`
- `MFCMouseEffect/MouseFx/Core/Overlay/NullOverlayCoordSpaceService.h`

能力：

- `SetOverlayWindowHandle(uintptr_t)`
- `ClearOverlayWindowHandle()`
- `SetOverlayOriginOverride(...)`
- `ClearOverlayOriginOverride()`
- `GetOverlayOrigin()`
- `ScreenToOverlayPoint(...)`

### 3.2 新增平台工厂

新增：

- `MFCMouseEffect/Platform/PlatformOverlayCoordSpaceFactory.h`
- `MFCMouseEffect/Platform/PlatformOverlayCoordSpaceFactory.cpp`

策略：

- Windows 返回 `Win32OverlayCoordSpaceService`；
- 其他平台暂返回 `nullptr`，由 Core 回落 `Null`。

### 3.3 Win32 实现下沉

新增：

- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayCoordSpaceService.h`
- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayCoordSpaceService.cpp`

说明：

- 迁移原子状态（overlay origin override）与虚拟屏原点读取逻辑；
- 由平台实现统一处理 Win32 坐标换算。

### 3.4 OverlayCoordSpace 改为委托服务

修改：

- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayCoordSpace.cpp`

关键变更：

- 移除本地 Win32 原子全局状态；
- 通过 `platform::CreateOverlayCoordSpaceService()` 获取服务；
- 工厂失败时回落 `NullOverlayCoordSpaceService`；
- 对外继续保留 `HWND/POINT` 签名，通过 `InputTypesWin32` 做边界转换。

### 3.5 工程清单同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增：

- `IOverlayCoordSpaceService`
- `NullOverlayCoordSpaceService`
- `PlatformOverlayCoordSpaceFactory`
- `Win32OverlayCoordSpaceService`

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 外部 API 未变，调用方无需批量修改；
- 服务失败时有 `Null` 回退，避免初始化硬失败。

当前边界：

- `OverlayCoordSpace.h` 仍暴露 Win32 类型（`HWND/POINT`）；
- 更彻底的平台无关 API 改造可后续分阶段推进。

## 6. 后续建议（阶段 14）

- 继续下沉仍位于 Core 的 Win32 overlay 组件（输入指示器）；
- 通过平台工厂注入，消除 `AppController` 对 Win32 overlay 具体类的直接依赖。
