# Scroll 滚轮样式优化（Arrow / Helix）

## 背景
新增了 Scroll 滚轮的第二种渲染风格 `helix`，但早期实现存在两个问题：

1. **代码层面不优雅**：`RenderParams`（方向/强度/是否循环）无法通过统一接口传递给 renderer，只能靠 `OnCommand("params", "...")` 这种字符串协议，导致可维护性差。
2. **视觉层面不精致**：`arrow` 与 `helix` 的发光层次、线条端点、缓动与漂移（drift）缺乏统一的“高级感”，整体显得生硬。

## 目标
- 让参数传递符合“面向接口编程”，避免散落的特殊处理。
- 让 `arrow` 与 `helix` 的滚动反馈更柔和、更有层次、更“优美”。

## 方案要点（重点）

### 1) 统一参数注入：`IRippleRenderer::SetParams`
在 `IRippleRenderer` 中新增可选接口：
- `virtual void SetParams(const RenderParams& params) {}`

这样 `ScrollEffect` / `RippleWindow` 可以以统一方式配置 renderer，而不需要：
- `dynamic_cast`
- `OnCommand("params", "...")` 字符串解析
- renderer 之间互相知道彼此的实现细节

### 2) 由窗口层负责“启动前”下发参数
`RippleWindow::StartContinuous(..., params)` 在调用 `Start(style)` 之前先：
- `renderer_->SetParams(params)`

并在 `UpdateRenderParams` 时同步刷新到 renderer，避免 window 保存了 `render_` 但 renderer 实际拿不到的问题。

### 3) 视觉优化策略（Arrow / Helix）

**Arrow（ChevronRenderer）**
- 增加两层 glow（外柔内紧），提升“光晕空气感”。
- 设置圆角端点（LineCapRound），减少锯齿和尖锐感。
- 增加轻微 drift，让反馈更“活”，但不过度飘动。

**Helix（HelixRenderer）**
- 统一缓动：采用更自然的 ease-out 衰减，避免突然消失。
- 使用 “aura + core” 双层线条：先画低 alpha 的外圈，再画内核线条。
- 头部高光更克制：小而亮的白色核心 + 柔和光晕，不刺眼。

## 涉及文件
- `MFCMouseEffect/MouseFx/Interfaces/IRippleRenderer.h`
- `MFCMouseEffect/MouseFx/Windows/RippleWindow.cpp`
- `MFCMouseEffect/MouseFx/Windows/RippleWindow.h`
- `MFCMouseEffect/MouseFx/Effects/ScrollEffect.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Scroll/ChevronRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Scroll/HelixRenderer.h`

## 手工验证建议
1. 在设置中切换 Scroll 类型：
   - `arrow`（方向指示）
   - `helix`（3D 双螺旋）
2. 分别测试：
   - 垂直滚动（delta 正/负）
   - 水平滚动（horizontal=true）
   - 轻滚（小强度）与快速滚动（高强度）
3. 观察点：
   - 光晕层次是否柔和、不糊成一团
   - 方向是否一致（上/下/左/右）
   - 高强度是否“更亮但不刺眼”

