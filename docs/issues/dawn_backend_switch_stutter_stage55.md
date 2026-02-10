# Dawn 切换卡顿收口（Stage 55）

## 目标

收紧 `CPU -> Dawn` 切换路径，避免在 Dawn 队列未就绪时提前重建全部特效，降低 `Apply` 与启动阶段的主线程卡顿。

## 问题现象

- 配置切到 `dawn/auto` 后，仍可能出现短时明显卡顿。
- 诊断日志常见：
  - `backend=cpu ... prime=not_attempted/backend_not_dawn`
  - 随后才进入 `backend=dawn ... queue_ready`
- 在队列尚未 ready 时，旧路径仍可能触发 `RecreateActiveEffects()`，导致无效重建。

## 关键调整

### 1) 后端偏好接口显式返回“是否应立刻重建”

- `OverlayHostService::SetRenderBackendPreference(...)` 从 `void` 改为 `bool`。
- 当目标为 `dawn/auto` 且当前为 CPU 且队列未 ready：
  - 不立即 `Shutdown()`
  - 返回 `false`
  - 通过 `backendDetail=dawn_probe_pending` 表示等待探测。

### 2) AppController 统一采用“队列就绪前不重建”

- `SetRenderBackend(...)`
- `ResetConfig()`
- `ReloadConfigFromDisk()`
- `WM_TIMER` 的 `deferredBackendApplyPending_` 分支

以上路径统一改为：

- 先读取 `SetRenderBackendPreference(...)` 返回值。
- `true` 才立即 `RecreateActiveEffects()`。
- `false` 且目标非 CPU 时，仅挂 `kDeferredBackendTimerId` 轮询等待 ready。

### 3) 桥接模式切换也避免无效重建

`SetGpuBridgeModeRequest(...)` 调整为：

- `renderBackend=cpu`：直接返回。
- 当前已是 Dawn 或队列已 ready：立即重建。
- 否则只挂延迟升级定时器，不做同步重建。

## 验证方式

1. `Release x64` 编译通过。
2. 启动后在 Web 设置中执行 `CPU -> Dawn` 切换。
3. 观察 `?diag=1`：
   - 队列未 ready 阶段应避免重复重建；
   - ready 后再进入 `accepted_*_cmd_submit`。
4. 体感应表现为：切换过程阻塞时间缩短，不再出现“未 ready 期间连续无效重建”。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
