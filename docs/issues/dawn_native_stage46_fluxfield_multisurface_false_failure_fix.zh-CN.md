# Stage 46 - FluxField 长按多屏无效修复（越界帧误判失败）

## 问题判定
- 结论：`Bug/回归`。
- 依据：
  1. `hold_fluxfield_gpu_v2` 路由已命中，且 GPU 计算可启动；
  2. 量子光环 GPU 正常，说明全局长按链路和输入事件链路正常；
  3. FluxField 在多 surface 渲染路径中，把“当前 surface 不需要绘制（目标在别的显示器）”当成失败，导致会话级视觉链路被提前熔断。

## 根因
- 文件：`MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.cpp`
- 旧逻辑对以下场景直接 `return false`：
  - `clip` 为空；
  - 目标矩形与当前 surface 无交集；
  - 可绘制宽高为 0。
- Overlay 是按显示器 surface 逐个渲染，同一个特效会在每个 surface 走一次 `Render`。  
  对于不包含鼠标点的 surface，这些场景是正常“跳过绘制”，不应计入失败。

## 修复方案
- 将“非目标 surface 的跳过帧”改为成功返回（no-op success）：
  - `GetClipBox == NULLREGION`：视为成功跳过；
  - `clipW/clipH <= 0`：视为成功跳过；
  - `rcRight <= rcLeft || rcBottom <= rcTop`：视为成功跳过。
- 保留真正错误路径（例如 `BindDC`/`EndDraw` 失败）为失败返回，用于真实故障熔断。

## 影响范围
- 仅影响 `hold_fluxfield_gpu_v2` 的 D2D 视觉后端失败判定。
- 不改变 Quantum Halo GPU 路由和其它长按/点击/拖尾特效行为。

## 回归验证
1. 选择长按特效：`磁通场 HUD GPU（全GPU）`。
2. 在主屏、扩展屏分别长按测试。
3. 预期：
   - 磁通场在当前鼠标所在屏幕可见；
   - 不再出现“量子光环有，磁通场完全无”的现象；
   - `x64/Release/.local/diag/flux_gpu_v2_compute_status_auto.json` 中 `route_reason` 维持 GPU 视觉可用状态，且不存在异常失败激增。
