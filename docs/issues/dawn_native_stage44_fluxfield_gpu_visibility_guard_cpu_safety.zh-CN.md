# Stage 44 - FluxField GPU 可见性护栏修复（D2D 成功但不可见场景）

## 问题判定
- 结论：`Bug/回归`。
- 依据：用户反馈 `hold_fluxfield_gpu_v2` 仍“和之前一样没效果”；运行日志无新的崩溃栈，但存在频繁 D3D 驱动装载/卸载，说明路由持续重建且可见性未达预期。

## 根因分析
- FluxField GPU 路径此前只在 `D2D Render 返回失败` 时才切 CPU。
- 在部分驱动/分层窗口组合下，D2D 渲染可能“返回成功但最终层合成不可见”，此时不会触发失败回退，用户感知就是“没效果”。

## 修复方案
- 文件：`MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- 策略调整为：`GPU 尝试 + CPU 可见性护栏`。
  1. 会话开始时始终启用 CPU 渲染护栏（保证可见输出）。
  2. GPU D2D 路径继续尝试渲染；若失败，降级为 CPU-only（本次会话）。
  3. 关闭该路由下不必要的 compute 启动，避免每次长按都触发重型驱动初始化噪音。

## 行为变化
- `hold_fluxfield_gpu_v2` 现在以“可见性优先”为第一目标：
  - 不再依赖“GPU 失败才有画面”；
  - 即使 GPU 路径在某些机型上不可见，CPU 护栏仍能稳定显示特效。

## 验证步骤
1. 选择长按特效：`磁通场 HUD GPU（CPU兜底）`。
2. 长按鼠标（可不移动）。
3. 预期：始终有可见特效，不再出现“完全无效果”。
4. 松开后效果正常结束。

## 构建验证
- 命令：
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect\MFCMouseEffect.vcxproj /m /p:Configuration=Release /p:Platform=x64`
- 结果：`0 error, 0 warning`。
