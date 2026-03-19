# Mouse Companion 指针朝向与连击发红契约（P2）

## 目标
- 对齐 `pet_companion` 原型的两项核心行为：
  - `face_pointer_enabled`（鼠标朝向跟随开关，默认关闭）
  - 连击驱动头部渐红（click streak + head tint）
- 统一配置、运行时、WebSettings、macOS Swift 视觉层与 proof 门禁。

## 判定
- 类型：`Design gap`（能力缺口补齐，不是单点补丁修复）。
- 依据：原型已定义开关与连击时序；主仓此前缺少完整端到端契约与可观测字段。

## 配置契约（mouse_companion）

### 生产档（默认）
- `face_pointer_enabled = false`
- `click_streak_break_ms = 650`
- `head_tint_per_click = 0.11`
- `head_tint_max = 0.70`
- `head_tint_decay_per_second = 0.36`

### 测试档（use_test_profile=true）
- `test_click_streak_break_ms = 1200`
- `test_head_tint_per_click = 0.20`
- `test_head_tint_max = 0.80`
- `test_head_tint_decay_per_second = 0.15`

### 取值范围（后端 sanitize）
- `click_streak_break_ms`: `[120, 3000]`
- `head_tint_per_click`: `[0.01, 1.0]`
- `head_tint_max`: `[head_tint_per_click, 1.0]`
- `head_tint_decay_per_second`: `[0.05, 4.0]`
- 测试档同范围。

## 行为契约

### 指针朝向开关
- `face_pointer_enabled=false`：
  - 移动不会触发 `follow` 自动动作。
  - 非拖拽结束态（如 `button_up/hover_end/hold_end`）回落为 `idle`。
- `face_pointer_enabled=true`：
  - 可触发 `follow`，并允许朝向跟随。
  - `follow` 不应在静止时锁住；应按指针速度自动回落到 `idle`。

### hover -> idle 映射
- `hover` 在 pet 语义里不再单独映射成独立动作名。
- `hover_start/hover_end` 仍保留为输入与插件诊断事件。
- 可见行为契约：
  - 停止输入达到 hover 阈值后，pet 进入增强版 `idle`；
  - 若开启 `face_pointer_enabled` 且移动速度重新超过 follow 阈值，则从 `idle` 自动切回 `follow`；
  - `fixed_bottom_left` 模式始终停留在 `idle` 路径，不因普通 move 自动切 `follow`。

### follow 速度选择与衰减
- 生产档：
  - `follow_speed_threshold_px_per_sec = 45`
  - `pointer_speed_decay_per_second = 8.5`
- 测试档（`use_test_profile=true`）：
  - `follow_speed_threshold_px_per_sec = 30`
  - `pointer_speed_decay_per_second = 6.0`
- 运行时规则：
  - 指针事件更新瞬时 `moveSpeed`
  - 帧循环按指数衰减 `moveSpeed`
  - `moveSpeed < 0.5` 直接归零
  - 仅 `moveSpeed > threshold` 时选择 `follow`，否则选择 `idle`

### 黑名单边界
- 常规鼠标特效仍遵循 app blacklist。
- `pet` 的 hover/idle 选择不应被该 blacklist 一起吞掉。
- 具体要求：
  - hover 定时器到达后，先进入 pet 的 `hover_start -> idle` 选择链；
  - 然后再决定是否拦截 hover wasm/native effect。

### 连击发红
- 仅主键点击进入连击统计。
- 相邻点击间隔 `<= click_streak_break_ms`：`streak + 1`，红度累加。
- 超阈值断连：仅清 `streak`，`tint` 不瞬间清零；按 `head_tint_decay_per_second` 逐帧回落。
- 点击叠加：`tint = min(tint + head_tint_per_click, head_tint_max)`。

## 运行时可观测字段（/api/state.mouse_companion_runtime）
- `click_streak`
- `click_streak_tint_amount`
- `click_streak_break_ms`
- `click_streak_decay_per_second`

## Test Dispatch 契约（/api/mouse-companion/test-dispatch）
- `runtime` 返回已对齐包含：
  - `click_streak`
  - `click_streak_tint_amount`
  - `click_streak_break_ms`
  - `click_streak_decay_per_second`
- 用途：Probe 面板/脚本在 test route 路径即可直接观测连击状态，不必额外查询 `/api/state`。

## WebSettings 对齐点
- Follow Tab：
  - `face_pointer_enabled` 开关
  - 生产档与测试档的 `click_streak_break_ms` / `head_tint_*` 输入
- Runtime Tab：
  - 连击段数、当前红度、断连阈值、褪红速度
- i18n：
  - EN/ZH 新增上述字段标签与开关文案键。
- fallback 路径（`WebUI/settings-form.js`）：
  - `mouse_companion` 的 normalize/read 兼容新字段，避免分片未挂载时 `Apply` 把新参数覆盖回旧值。

## macOS 视觉层契约
- C bridge `mfx_macos_mouse_companion_update_state_v1` 新增 `headTintAmount` 参数。
- Swift host 按头部材质关键字匹配（`head/face/kao/头/脸/顔`），基于基色到红色线性混合并缓存，避免重复重算。

## 回归步骤
1. 打开 `Mouse Companion -> Follow`，确认可见 `face_pointer_enabled` 与连击/红度参数（含测试档）。
2. 默认 `face_pointer_enabled=false`，移动鼠标，确认 Runtime 动作不被稳定驱动到 `follow`。
3. 连续点击 3~6 次（小于断连阈值），确认 `click_streak` 增长，`head_tint_amount` 增强。
4. 停止点击超过断连阈值，确认 `click_streak` 清零且 `head_tint_amount` 渐退（非瞬清）。
5. 开启 `use_test_profile`，使用测试值重复步骤 3~4，确认更易观察。

## 自动化门禁
- 命令：
  - `tools/platform/manual/run-macos-mouse-companion-proof.sh --skip-webui-build`
- 当前 proof 载荷显式设置 `face_pointer_enabled=false`，动作序列断言回落到 `idle` 分支。
- proof 额外断言连击契约：
  - `click_streak >= 1`
  - `click_streak_tint_amount > 0`
  - `click_streak_break_ms = 650`
  - `click_streak_decay_per_second > 0`
