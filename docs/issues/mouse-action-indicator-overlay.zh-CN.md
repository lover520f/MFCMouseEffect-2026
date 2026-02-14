# 鼠标动作指示器叠层

## 背景
需要在不影响正常鼠标操作的前提下，在光标附近可视化点击与滚轮动作。

## 本次新增
- 新运行时组件：`MouseActionIndicator`
  - 文件：
    - `MFCMouseEffect/MouseFx/Core/MouseActionIndicator.h`
    - `MFCMouseEffect/MouseFx/Core/MouseActionIndicator.cpp`
- 覆盖事件：
  - 左键单击/双击/三击
  - 右键单击/双击/三击
  - 滚轮上/下
- 渲染方式：
  - 分层透明、点击穿透、无激活窗口
  - GDI+ ARGB 绘制 + 定时器动画

## 定位模式
- 相对模式：
  - 以光标为锚点，使用 `offset_x / offset_y`
- 绝对模式：
  - 使用虚拟桌面坐标 `absolute_x / absolute_y`
  - 多屏场景下按全局屏幕坐标生效

## 配置与持久化
- `EffectConfig` 新增：
  - `mouse_indicator.enabled`
  - `mouse_indicator.position_mode`
  - `mouse_indicator.offset_x`, `offset_y`
  - `mouse_indicator.absolute_x`, `absolute_y`
  - `mouse_indicator.size_px`
  - `mouse_indicator.duration_ms`
- 读写实现：
  - `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`
- 默认值策略：
  - 对于未包含 `mouse_indicator` 的旧配置，默认按 `enabled=true` 处理，确保功能开箱可见。

## 运行时接入
- `AppController` 在以下消息中分发指示器事件：
  - `WM_MFX_CLICK`
  - `WM_MFX_SCROLL`
- 与 VM 前台抑制联动：
  - 进入 VM 前台抑制时隐藏指示器

## 黑色方块问题修复
- 现象：指示器窗口显示为黑色方块，内部动画不可见。
- 根因：
  - `SetLayeredWindowAttributes` 与 `UpdateLayeredWindow` 混用，导致分层更新失败。
  - 使用 `GDI+ GetHBITMAP` 路径时 alpha 信息不可靠，透明背景可能退化。
- 修复：
  - 移除 `SetLayeredWindowAttributes`。
  - 改为 `CreateDIBSection(32bit ARGB) + GDI+ 直接绘制 + UpdateLayeredWindow` 的单一路径。
  - 在 Debug 输出中增加 `UpdateLayeredWindow` 失败错误码，便于后续排查。

## 增量修复：滚轮识别 / 键盘支持 / 视觉越界
- 滚轮与中键识别：
  - `MouseActionIndicator::OnClick` 去掉"未知按钮默认按左键"的兜底逻辑，避免错误显示为 `L`。
  - `GlobalMouseHook` 补充双击类鼠标消息映射（`WM_*BUTTONDBLCLK`）并统一滚轮 delta 判定（忽略 `delta=0`）。
- 键盘支持：
  - `GlobalMouseHook` 新增 `WH_KEYBOARD_LL`，在 `WM_KEYDOWN/WM_SYSKEYDOWN` 时投递 `WM_MFX_KEY`。
  - `AppController` 新增 `WM_MFX_KEY` 分发到 `MouseActionIndicator::OnKey`。
  - Web 设置新增 `mouse_indicator.keyboard_enabled` 开关，支持读/写/持久化全链路。
- 视觉与越界：
  - 指示器按钮高亮改为"圆角路径内裁剪绘制"，避免出现矩形高亮溢出圆角外部。
  - 位置钳制继续基于虚拟桌面坐标，防止窗口超出屏幕有效区域。
  - 标签压缩为短文本（如 `L2/R2/M2/W+`），减少小尺寸下文本拥挤。

## Web 设置接入
- 后端 schema/state：
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- 前端控件：
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUI/app.js`
- 支持项：
  - 开关
  - 定位模式
  - 相对/绝对坐标
  - 指示器大小与动画时长

## 架构重构：IndicatorRenderer 提取
- 所有 GDI+ 渲染逻辑从 `MouseActionIndicator` 提取至独立类：
  - `MFCMouseEffect/MouseFx/Renderers/Indicator/IndicatorRenderer.h`
  - `MFCMouseEffect/MouseFx/Renderers/Indicator/IndicatorRenderer.cpp`
- `MouseActionIndicator.cpp` 改进：
  - 使用 RAII `GdiRenderContext` 防止 GDI 资源泄漏。
  - 所有绘制委托给 `IndicatorRenderer::RenderMouseAction()` / `RenderKeyAction()`。
  - 窗口类注册使用 bool 守卫，避免重复注册。
- `IndicatorRenderer` 视觉增强：
  - 点击涟漪：左/右/中键点击时显示向外扩散的半透明光圈。
  - 滚轮箭头：加大尺寸，增加拖尾渐变效果。

## 键盘组合键支持
- `KeyEvent` 新增修饰键状态字段（`ctrl`、`shift`、`alt`、`win`）。
- `GlobalMouseHook::KeyboardHookProc` 通过 `GetAsyncKeyState` 检测修饰键。
- 显示逻辑：
  - 组合键：显示为 `Ctrl+C`、`Alt+Tab`、`Ctrl+Shift+S`。
  - 单独按修饰键：仅显示修饰键名（如 `Ctrl`），不带尾部 `+`。

## 滚轮连续计数
- 连续滚动：在 500ms 内同向滚动时，会在滚轮标签后追加次数（如 `W+ 3`、`W- 5`）。
- 重置条件：反向滚动或超过 500ms 无操作。

## 验证清单
1. 在 Web 设置页开启指示器并应用。
2. 验证左/右键单击、双击、三击动画（含点击涟漪）。
3. 验证滚轮上/下动画（含加大箭头）。
4. 在相对/绝对模式间切换并验证位置变化。
5. 在多屏环境验证绝对坐标在虚拟桌面坐标系下正确生效。
6. 前台进入虚拟机窗口时，确认指示器与其他特效一起被抑制。
7. 在 Web 设置切换"启用键盘指示"，验证按键时显示/隐藏行为符合开关状态。
8. 按 `Ctrl+C`，验证显示为 "Ctrl+C"。
9. 单独按 `Ctrl`，验证显示为 "Ctrl"（非 "Ctrl+"）。
