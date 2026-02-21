# FluxField 长按无效修复（hold_state 生命周期解耦）

## 问题判定
- 结论：`Bug/回归`。
- 依据：仅 `hold_fluxfield_gpu_v2` 路由异常，其他长按特效正常，说明全局长按触发链路有效，问题集中在 FluxField 路由内部状态机。

## 现象
- 在 Web 设置选择 `磁通场 HUD GPU（CPU兜底）` 后，长按没有可见特效。
- 其他长按特效（如 charge/lightning/neon3d/quantum）可正常显示。

## 根因
- FluxField 路由用 `hold_state` 命令传递 `{hold_ms,x,y}`，但历史实现把 `hold_ms == 0` 同时当作：
  - 长按开始的首帧状态（合法）；
  - 长按结束信号（清空状态并写快照）。
- 这导致渲染器在启动初始帧就可能把光标状态清空。
- 当用户“按住不动”时，后续位移更新稀疏，渲染锚点状态不稳定，进而出现无可见效果/位置异常。

## 修复方案

### 1) 运行时协议去歧义
- 文件：`MFCMouseEffect/MouseFx/Effects/RippleBasedHoldRuntime.h`
- 文件：`MFCMouseEffect/MouseFx/Effects/RippleBasedHoldRuntime.cpp`
- 变更：新增 `SendHoldEndCommand`，在 `Stop()` 阶段发送独立命令 `hold_end`，不再复用 `hold_state(0,...)` 表示结束。

### 2) Flux 渲染器状态机修正
- 文件：`MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- 变更：
  - `hold_state`：统一视为“进行中状态更新”，保留 `cursor` 状态，不再因 `ms==0` 清空。
  - `hold_end`：专职执行结束清理（`active=false`、`holdMs=0`、清空 cursor）并写诊断快照。

## 影响范围
- 仅影响 FluxField GPU V2 长按路由状态管理。
- 不改变其他长按特效、点击特效、WASM 插件链路。

## 回归验证
1. 选择长按特效为 `磁通场 HUD GPU（CPU兜底）`。
2. 鼠标左键长按（保持鼠标尽量不移动）超过触发阈值。
3. 预期：出现可见磁通场效果，不再出现“无效果”状态。
4. 释放后效果结束；重复多次行为稳定。

## 构建验证
- 命令：
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect\MFCMouseEffect.vcxproj /m /p:Configuration=Release /p:Platform=x64`
- 结果：`0 error, 0 warning`。
