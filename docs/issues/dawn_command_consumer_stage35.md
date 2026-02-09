# Dawn 命令消费闭环（Stage 35）

## 目标
- 在不改变现有 CPU 渲染结果的前提下，打通：
  - 图层命令流生成
  - Dawn 消费层接收
  - Web 侧可观测诊断

## 新增
- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
  - `SubmitOverlayGpuCommands(...)`
  - `GetDawnCommandConsumeStatus()`
  - `ResetDawnCommandConsumeStatus()`

## 接线
1. OverlayHostWindow 每帧提交
- 文件：`MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
- 在 `CollectGpuCommandStream(...)` 末尾调用 `SubmitOverlayGpuCommands(...)`。

2. 后端上下文传入
- 文件：`MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h/.cpp`
- 新增 `SetGpuSubmitContext(activeBackend, pipelineMode)`。
- `OverlayHostService::Initialize()` 创建窗口后注入当前后端/管线上下文。

3. 生命周期复位
- 文件：`MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
- `Shutdown()` 时调用 `ResetDawnCommandConsumeStatus()`，避免旧状态污染。

## 状态透出
- 文件：`MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `/api/state` 新增 `dawn_command_consumer`：
  - `accepted`
  - `detail`
  - `accepted_frames/rejected_frames`
  - 各命令计数

- 文件：`MFCMouseEffect/WebUI/app.js`
- `?diag=1` 下横幅追加消费状态：
  - `消费: 已接收/未接收 (detail)`

## 当前行为边界
- Stage 35 为“消费闭环 + 诊断可见”，暂未实现真实 Dawn DrawPass。
- 当 `activeBackend != dawn`、非 `dawn_compositor`、桥接未就绪等场景会明确拒绝并给出 `detail`。
- `accepted_stub_noop` 表示命令已被消费层接收，下一阶段接入真实 GPU 绘制。

