# 多屏渲染裁剪优化（长按/拖尾/悬浮）

## 背景
- 当前 Overlay 架构为每个显示器创建一个分层窗口。
- 每一帧会对每个窗口重复执行所有图层 `Render(...)`。
- 在多屏（尤其高分屏）下，长按/拖尾/悬浮这类重特效会被重复绘制，导致明显 CPU 放大和掉帧。

## 本次改动
1. 扩展图层接口，增加按屏幕区域裁剪能力
- 文件：`MFCMouseEffect/MouseFx/Interfaces/IOverlayLayer.h`
- 新增默认方法：
  - `IntersectsScreenRect(int left, int top, int right, int bottom) const`
  - 默认返回 `true`，保证旧图层行为不变。

2. 在 OverlayHostWindow 渲染时按屏幕做图层过滤
- 文件：`MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
- 在 `RenderSurface(...)` 中，仅当图层与当前屏幕窗口相交时才调用 `layer->Render(...)`。
- 对“当前无可见内容且上一帧也为空”的屏幕，直接跳过：
  - `ZeroMemory` 清屏
  - `UpdateLayeredWindow`
  仅在“从有内容 -> 无内容”的切换帧执行一次清空提交，避免残影并减少重复提交开销。

3. 为重负载图层实现相交判断（重点覆盖用户反馈场景）
- 拖尾：`MFCMouseEffect/MouseFx/Layers/TrailOverlayLayer.h/.cpp`
  - 基于历史轨迹点计算包围盒并加安全 padding。
  - 当点集为空时回退使用最后采样点，避免尾迹渐隐阶段被误裁剪。
- 长按/悬浮（同属 Ripple 管线）：`MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h/.cpp`
  - 基于每个 active 实例中心点和 `windowSize` 计算包围区域并加入 glow 余量。
- 粒子拖尾：`MFCMouseEffect/MouseFx/Layers/ParticleTrailOverlayLayer.h/.cpp`
  - 基于粒子集合包围盒判断，作为拖尾家族补齐。

## 预期效果
- 多屏下，特效只在相关屏幕绘制，不再在所有屏幕重复做重计算/重绘制。
- 对无活跃特效的副屏，减少逐帧 `UpdateLayeredWindow` 提交。
- 对长按、拖尾、悬浮三类卡顿场景有直接缓解。
- 对单屏行为无功能性变化。

## 验证
- `MSBuild ... /t:ClCompile /p:Configuration=Release /p:Platform=x64` 通过。
- 全量链接失败原因为目标 `MFCMouseEffect.exe` 被占用（`LNK1104`），非编译错误。

## 后续建议（衔接 GPU 迁移）
- 本次裁剪属于 CPU/GDI+ 路径减负。
- 下一阶段可在此基础上推进：
  - 为长按/拖尾/悬浮输出统一“GPU 可消费的渲染命令（几何+材质参数）”；
  - Dawn 后端直接消费命令，CPU 路径继续兜底。
