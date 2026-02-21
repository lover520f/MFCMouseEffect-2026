# Stage 45 - FluxField 恢复全量 GPU 并修正 D2D 可见区域裁剪

## 问题判定
- 结论：`设计行为偏离需求 + 显示Bug`。
- 依据：
  1. 用户明确要求 `hold_fluxfield_gpu_v2` 必须为全量 GPU（GPU 计算 + GPU 渲染）。
  2. 现网实现被临时改为可见性护栏（CPU 保底），不满足“全量 GPU”约束。
  3. 历史“看不到效果”与 D2D 绑定区域/裁剪边界计算存在环境相关风险。

## 目标
- 在不引入 CPU 渲染保底的前提下，恢复 `hold_fluxfield_gpu_v2` 纯 GPU 路径。
- 提升 D2D 绘制区域计算稳定性，避免“渲染成功但画到不可见区域”。

## 核心修改

### 1) FluxField 路由恢复纯 GPU
- 文件：`MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- 变更：
  - `Start()` 恢复 `gpuCompute_.Start()`（重新启用 GPU 计算）。
  - 关闭 CPU fallback（`cpuFallbackActive_=false`，不再调用 CPU 渲染器）。
  - `Render()` 仅在 `gpuStarted_ && gpuCompute_.IsActive() && gpuVisualActive_` 下执行。
  - GPU 视觉失败时仅标记失败并写快照，不切到 CPU。

### 2) D2D 可见区域裁剪修正
- 文件：`MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.cpp`
- 变更：
  - 移除基于 `GetDeviceCaps(HORZRES/VERTRES)` 的边界判定。
  - 改为基于当前 HDC 的 `GetClipBox()` 作为真实可绘制区域。
  - 光标重定位改为禁用，统一使用矩阵锚点（由上游 Ripple 层提供），避免双重坐标系偏移。
  - `BindDC` 改为绑定整块 `clip box`，避免子矩形绑定下不同驱动对坐标原点解释不一致导致“渲染成功但不可见”。

### 3) 设置文案与实现一致化
- 文件：`MFCMouseEffect/Settings/SettingsOptions.h`
- 文件：`MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
- 变更：
  - 将显示文案从 `CPU兜底 / CPU Fallback` 调整为 `全GPU / Full GPU`，避免与当前实现不一致。

### 4) 诊断可观测性增强
- 文件：`MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- 变更：
  - 在快照中新增 `render_attempt_count / render_success_count / render_failure_count`，
    用于区分“未进入渲染”与“进入渲染但不可见”。

## 影响范围
- 仅影响 `hold_fluxfield_gpu_v2` 路由。
- 不影响其他长按特效、点击/拖尾/滚轮/悬停链路。

## 验证步骤
1. 选择长按特效：`磁通场 HUD GPU（全GPU）`。
2. 长按鼠标并观察效果是否稳定可见。
3. 检查诊断文件：
   - `x64/Release/.local/diag/flux_gpu_v2_compute_status_auto.json`
   - 预期：`gpu_started=true`、`gpu_active=true`、`cpu_fallback_active=false`。

## 构建验证
- 命令：
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect\MFCMouseEffect.vcxproj /m /p:Configuration=Release /p:Platform=x64`
- 结果：`0 error, 0 warning`。
