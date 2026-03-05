# 效果协同 Bug 修复记录

## 概述
修复了 3 个效果协同（Effect Synergy）交互 Bug，均涉及长按效果与其他效果的协同策略。

## Bug 1: Hold+Move Trail 断裂
- **现象**：`hold_move=move_only` 时，长按并移动，拖尾第一段断裂
- **根因**：`kTrailInputGapResetMs=220` 间隔重置后 `return` 跳过当前帧。Hold delay 350ms > gap 阈值 220ms → 必定触发
- **修复**：移除 gap-reset 中的 `return`，改为 fall-through。`lastPoint_=pt` 后 `moveDx=0` → 不发射（安全），下一帧正常

## Bug 2: Hold+Scroll 惯性滚动持续
- **现象**：`hold_scroll` 策略下，停止滚轮后仍有滚轮特效
- **修复**：`HandleScrollEvent` 过滤 `kCGScrollWheelEventMomentumPhase != 0`（对触控板/Magic Mouse 有效）
- **注意**：物理鼠标滚轮无 momentum 事件，"持续"是 overlay 动画持续时间（正常行为）

## Bug 3: move_only 导致 Hold 全局不可用
- **现象**：`hold_move=move_only` 时，Hold 在所有协同场景（hover/scroll/click）都不显示
- **根因**：`ShouldSuppressHoldStartForMoveOnlyPolicy` 在 hold timer 中完全阻止 hold 创建
- **修复**：
  1. 移除 `ShouldSuppressHoldStartForMoveOnlyPolicy` 检查 → hold 始终创建
  2. `EndHoldEffectLaneIfNeeded` 不再 disarm update timer → 移动停止后 timer 可重启 hold
  3. 新增 `IMouseEffect::IsEffectActive()` → 检测 hold overlay 是否存活
  4. Timer handler 增加 cursor idle 检测（6 tick ≈100ms 静止后重启，防闪烁）

## 其他
- `.gitignore` 增加 `build-macos/`
