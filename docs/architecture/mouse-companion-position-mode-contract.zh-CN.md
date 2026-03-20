# Mouse Companion 位置模式契约（P2）

## 目标
- 将 `mouse_companion` 的位置配置对齐到 `Input Indicator`：位置模式只表达“pet 放哪儿”，不再混入动作语义。
- 支持两类主模式：
  - `relative`：相对鼠标位置
  - `absolute`：绝对屏幕坐标
- 保留旧配置兼容：
  - `fixed_bottom_left`
  - `follow`

## 判定
- 类型：`Design behavior`
- 依据：`mouse_companion.positionMode` 之前同时承担“窗口位置策略”和“动作 follow 语义”，已经不适合当前 mac 收尾目标；`Input Indicator` 已经有更清晰的 `relative / absolute / target_monitor` 契约，可以直接复用。

## 配置契约（mouse_companion）

### 主字段
- `position_mode`
  - 推荐值：
    - `relative`
    - `absolute`
  - 兼容值：
    - `fixed_bottom_left`
    - `follow`
- `offset_x`
- `offset_y`
- `absolute_x`
- `absolute_y`
- `target_monitor`

### 默认值
- 当前后端默认仍保持 `fixed_bottom_left`
  - 目的：不打断既有配置与已验证的 mac 行为
- 新 UI/新契约优先展示 `relative / absolute`

### sanitize 规则
- `position_mode`
  - 接受：`relative | absolute | fixed_bottom_left | follow`
  - 非法值回退到 `fixed_bottom_left`
- `absolute_x / absolute_y`
  - clamp 到 `[-20000, 20000]`
- `target_monitor`
  - 空值回退到 `cursor`
  - 存储时统一 trim + lowercase

## 行为契约

### `relative`
- pet 面板相对当前鼠标位置显示
- `offset_x / offset_y` 生效
- mac 宿主使用当前 `NSEvent.mouseLocation`
- 面板原点会经过 `edge_clamp_mode` 处理：
  - `strict`：完整限制在桌面 union 内，底边/顶边按 `NSScreen.frame` 处理，不受 Dock / menu bar 工作区缩减影响
  - `soft`：允许部分越界，但至少保留一部分面板可见
  - `free`：不做桌面 clamp

### `absolute`
- pet 面板使用目标屏幕的绝对坐标
- 坐标语义与 `Input Indicator` 保持一致：
  - `absolute_x`：目标屏幕左上角起点的 X
  - `absolute_y`：目标屏幕左上角起点的 Y
- mac 宿主内部会转换为 Cocoa 左下角窗口原点：
  - `origin.y = screen.maxY - absolute_y - panel.height`
- 绝对坐标计算完成后同样会经过 `edge_clamp_mode` 处理
- mac Swift 宿主会覆盖 `NSPanel.constrainFrameRect`，避免 AppKit 再把窗口静默拉回 `visibleFrame`

### `target_monitor`
- 当前 mac 宿主支持：
  - `cursor`
  - `primary`
  - 显式 monitor id / `monitor_<id>`
- `custom` 仍作为 legacy passthrough 接受，但当前 pet UI 不主推该值

### 兼容模式

#### `fixed_bottom_left`
- 仅作为 legacy compatibility 保留
- 继续锚定到桌面联合边界左下角
- 便于保留当前已验证的 click/idle/hold/scroll/follow 行为
- 计算出的原点同样会经过 `edge_clamp_mode` 处理

#### `follow`
- 仅作为 legacy compatibility 保留
- 位置语义映射到 `relative`

## 端到端对齐范围
- `EffectConfig` 结构、JSON parse/serialize、sanitize
- `/api/state` schema/state 映射
- WebSettings `Mouse Companion -> Follow` 面板读写
- macOS Swift C bridge `mfx_macos_mouse_companion_panel_configure_v1`
- 面板尺寸变化后的重新定位：
  - `relative` 保持中心
  - `absolute` 重算目标屏幕绝对锚点
  - `fixed_bottom_left` 保持 legacy 左下角锚点

## WebUI 契约
- 前端入口改为单面板轻表单，不再使用 `Basic / Follow / Probe / Runtime` 多页签。
- `Position Mode` 下拉优先展示：
  - `relative`
  - `absolute`
- legacy 选项仍显示，但标记为兼容用途
- `Edge Clamp Mode` 下拉显式暴露：
  - `soft`
  - `free`
  - `strict`
- 表单显隐规则：
  - `relative/follow`：高亮 `offset_x / offset_y`
  - `absolute`：高亮 `absolute_x / absolute_y` 与 `target_monitor`
  - `fixed_bottom_left`：保留 legacy 值，不自动强迁移
- 用户日常仅暴露：
  - 基础开关与尺寸
  - `Placement`
  - `Asset Paths`
  - `Advanced Motion`
- 开发态 `Probe / Runtime` 面板与对应前端控制器已从用户页面移除。
- `use_test_profile` 与 `test_*` 配置仍保留在后端契约里，但不再出现在默认用户面板；表单提交时会保留已有值，不会因为字段隐藏而被重置。

## 回归步骤
1. 打开 `Mouse Companion` 设置面板，确认页面为单面板布局，且不再出现 `Probe / Runtime` 页签。
2. 在 `Placement` 中确认 `Position Mode` 出现 `relative / absolute`，且 legacy 选项仍可见。
3. 选 `relative`，设置 `offset_x / offset_y` 后 `Apply`，确认 pet 相对鼠标偏移显示。
4. 选 `absolute`，设置 `absolute_x / absolute_y` 与 `target_monitor` 后 `Apply`，确认 pet 固定到目标屏幕绝对坐标。
5. 切换 `edge_clamp_mode`：
   - `strict`：pet 不能越出桌面 union
   - `soft`：pet 可部分越界，但仍保留可见面积
   - `free`：pet 可越界，不再被强制拉回屏幕内
6. 旧配置值 `fixed_bottom_left / follow / use_test_profile / test_*` 回读后再次 `Apply`，不出现配置丢失或被默认值覆盖。
