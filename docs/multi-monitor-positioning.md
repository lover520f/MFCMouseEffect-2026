# 多屏幕支持 + 键盘独立定位 + 每屏幕位置覆盖

## 背景

InputIndicator 原本仅支持相对/绝对两种定位。
多屏幕环境下，用户希望：
1. **目标屏幕**：指示器能固定在某一个屏幕，或者跟随鼠标所在的屏幕。
2. **键盘独立**：键盘指示器可以与鼠标指示器分离，设置不同的位置。
3. **每屏幕独立坐标**：在绝对定位模式下，跟随鼠标时，每个屏幕可以有不同的绝对坐标（例如屏幕分辨率不同导致的位置差异）。

## 核心功能

### 1. 目标屏幕 (Target Monitor)
通过 `target_monitor` 指定：
- `"cursor"` (默认)：指示器出现在**当前光标所在**的屏幕。
- `"primary"`：强制固定在主屏幕。
- `"monitor_N"`：强制固定在第 N 个屏幕。

### 2. 键盘独立定位
- `keyboard_follow_mouse`: `true` 时，键盘指示器完全使用鼠标的定位参数。
- `keyboard_follow_mouse`: `false` 时，键盘使用独立的 `kb_*` 参数（模式、偏移、绝对坐标、目标屏幕）。

### 3. 每屏幕位置覆盖 (Per-Monitor Overrides)
仅在 `position_mode = "absolute"` 且 `target_monitor = "cursor"` 时生效。
系统优先查找当前屏幕是否有**覆盖配置**，如果有则使用覆盖的坐标，否则使用全局默认坐标。

**配置示例**：

```json
{
  "input_indicator": {
    "position_mode": "absolute",
    "absolute_x": 40,          // 全局默认
    "absolute_y": 40,
    "target_monitor": "cursor", // 跟随鼠标屏幕
    "per_monitor_overrides": {
      "monitor_2": { "absolute_x": 100, "absolute_y": 80 } // 屏幕2单独设置
    },
    
    // 键盘独立配置
    "keyboard_follow_mouse": false,
    "kb_position_mode": "absolute",
    "kb_per_monitor_overrides": {
       "monitor_1": { "absolute_x": 200, "absolute_y": 200 }
    }
  }
}
```

## 实现细节

### 数据结构
- `EffectConfig.h`: `PerMonitorPosOverride` { x, y }
- Map: `monitor_id` -> `PerMonitorPosOverride`

### 逻辑流程
1. `InputIndicatorOverlay::UpdatePlacement` 获取当前光标位置。
2. 调用 `MonitorUtils::ResolveTargetMonitor` 获取屏幕 ID。
3. 检查 Config 中是否存在该 ID 的 override。
4. 存在则应用，不存在则使用默认 `absoluteX/Y`。
5. 坐标基于该屏幕的左上角进行偏移。

### 前端交互
Web 设置页会自动枚举当前连接的所有显示器。在绝对模式下，会显示 "Per-Monitor Overrides" 区域，允许用户为每个屏幕单独输入 X/Y 坐标。留空则表示使用默认值。
