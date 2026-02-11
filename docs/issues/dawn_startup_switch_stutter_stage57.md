# Dawn 启动/切换卡顿收紧（Stage 57）

## 目标

处理两类卡顿：

1. `auto -> dawn` 仅偏好切换时仍触发全量重建导致顿挫。
2. Dawn 队列就绪后立即在高频鼠标输入中重建，导致鼠标明显掉帧。

## 改动

### 1) 去掉 Dawn 偏好等价切换的无效重建

在 `OverlayHostService::SetRenderBackendPreference(...)` 中新增判定：

- 当前 `activeBackend == dawn`
- 且 `requested` 与 `new requested` 同属 `auto/dawn` 族

此时只更新偏好，不再 `Shutdown()`，避免 `auto <-> dawn` 时无意义的重建/卡顿。

### 2) 引入“输入空闲窗口”再执行重建

`AppController` 增加：

- `IsBackendSwitchIdleWindow()`
- 阈值 `kDeferredBackendIdleThresholdMs = 240`

在以下路径中，若目标是 Dawn 且当前输入仍活跃，则改为延迟重建（80ms 重试）：

- `SetRenderBackend(...)` 的立即重建路径
- `WM_TIMER` 中 `deferredBackendApplyPending_` 分支
- `WM_TIMER` 中 `deferredDawnUpgradePending_` 且 queue ready 分支

如果持续繁忙超过重试上限，仍会执行重建，避免永久不切换。

## 结果预期

- `auto -> dawn`（且当前已在 Dawn）不再出现大卡顿。
- 启动/切换阶段重建尽量避开鼠标高频输入窗口，体感更平滑。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
- `MFCMouseEffect/MouseFx/Core/AppController.h`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
