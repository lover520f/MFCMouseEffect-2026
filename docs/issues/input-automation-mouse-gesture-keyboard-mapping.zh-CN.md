# 鼠标 Action / 手势 映射到键盘（阶段一）

## 背景
当前项目已具备全局鼠标事件采集和输入指示器能力。本阶段在不破坏现有特效架构的前提下，新增输入自动化能力：
- 鼠标 action -> 键盘快捷键
- 鼠标手势 -> 键盘快捷键

目标是先打通稳定的核心链路（采集、识别、映射、注入、配置持久化），并保持默认关闭，避免影响现有用户行为。

## 架构落点

### 1. `Core/Input`（识别层）
- 新增：`MFCMouseEffect/MouseFx/Core/Input/GestureRecognizer.h`
- 新增：`MFCMouseEffect/MouseFx/Core/Input/GestureRecognizer.cpp`
- 职责：
  - 在按键按下-移动-抬起期间采样轨迹
  - 将轨迹离散为方向序列
  - 输出统一 gesture id（如 `up`、`left_right`）

### 2. `Core/Automation`（映射与执行层）
- 新增：`MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.h`
- 新增：`MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.cpp`
- 新增：`MFCMouseEffect/MouseFx/Core/Automation/KeyChord.h`
- 新增：`MFCMouseEffect/MouseFx/Core/Automation/KeyChord.cpp`
- 新增：`MFCMouseEffect/MouseFx/Core/Automation/KeyboardInjector.h`
- 新增：`MFCMouseEffect/MouseFx/Core/Automation/KeyboardInjector.cpp`
- 职责：
  - 根据配置查找匹配映射规则
  - 解析快捷键文本（如 `Ctrl+Shift+S`）
  - 调用 `SendInput` 注入键盘事件
  - 手势触发后自动抑制同次按钮抬起产生的 click 映射，避免重复触发

### 3. `Core/Control`（编排层）
- `DispatchRouter` 中接入 `InputAutomationEngine`：
  - click / scroll 动作映射
  - button down / move / button up 手势链路
- `AppController` 新增自动化配置生命周期管理：
  - 启动加载
  - 运行时更新
  - VM 抑制时 reset

## 配置模型（`config.json`）
新增根字段：`automation`

```json
{
  "automation": {
    "enabled": false,
    "mouse_mappings": [
      { "enabled": true, "trigger": "left_click", "keys": "Ctrl+C" }
    ],
    "gesture": {
      "enabled": false,
      "trigger_button": "right",
      "min_stroke_distance_px": 80,
      "sample_step_px": 10,
      "max_directions": 4,
      "mappings": [
        { "enabled": true, "trigger": "up_right", "keys": "Alt+Tab" }
      ]
    }
  }
}
```

已完成：
- 读取/写回（parse/serialize）
- `apply_settings` 运行时应用
- settings state/schema 输出（便于 WebUI 后续直接接控件）

## 当前支持

### 鼠标 action trigger
- `left_click`
- `right_click`
- `middle_click`
- `scroll_up`
- `scroll_down`

### 手势 trigger（识别输出）
- 单方向：`up` / `down` / `left` / `right`
- 组合方向：如 `up_right`、`down_left`、`left_up_right`

### 快捷键解析
- 修饰键：`Ctrl` / `Shift` / `Alt` / `Win`
- 主键：`A-Z`、`0-9`、`F1-F24`、方向键、`Tab`、`Enter`、`Esc`、`Space`、`Delete` 等常用键

## 兼容性与默认行为
- 默认 `automation.enabled=false`，老用户升级后行为不变。
- 新字段缺失时会回落默认值并自动 sanitize。
- 输入自动化与现有 click/trail/hold/indicator 共存，不替代原有渲染链路。

## 验证步骤（手工）
1. 启动程序，确认常规鼠标特效正常。
2. 在 `config.json` 开启 `automation.enabled=true`，配置 `mouse_mappings`（例如 `left_click -> Ctrl+C`）。
3. 点击鼠标验证目标程序收到快捷键。
4. 开启 `gesture.enabled=true`，配置手势映射（例如 `up_right -> Alt+Tab`）。
5. 按住配置按钮（默认右键）拖出手势后松开，验证快捷键触发。
6. 切换 VM 前台抑制场景，确认抑制时不触发，恢复后正常。

## 后续建议（阶段二）
1. WebUI 增加可视化映射编辑器（增删规则、冲突提示、快捷键录制）。
2. 增加手势可视化调试（采样点和方向链路日志，默认关闭）。
3. 可选“阻断原始鼠标行为”策略（需谨慎，避免破坏系统交互）。
