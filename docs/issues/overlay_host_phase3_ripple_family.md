# OverlayHost Phase 3: Ripple Family 迁移

## 1. 范围

本阶段把以下效果统一迁移到 `OverlayHostWindow` 的同一画板：

- click `RippleEffect`
- scroll `ScrollEffect`
- hold `HoldEffect`
- hover `HoverEffect`

`text` 暂未迁移，仍沿用 `TextWindowPool`。

## 2. 技术改动

### 2.1 新增 Ripple 图层

- `MouseFx/Layers/RippleOverlayLayer.h/.cpp`

能力：

- 支持 one-shot 实例（click/scroll）
- 支持 continuous 实例（hold/hover）
- 支持按 id 更新位置、停止、发命令、广播命令

### 2.2 扩展 Host 服务

- `MouseFx/Core/OverlayHostService.h/.cpp`

新增接口：

- `ShowRipple(...)`
- `ShowContinuousRipple(...)`
- `UpdateRipplePosition(...)`
- `StopRipple(...)`
- `SendRippleCommand(...)`
- `BroadcastRippleCommand(...)`

### 2.3 效果层接入策略

- `RippleEffect` / `ScrollEffect`：优先 Host，失败时动态回退 `RippleWindowPool`
- `HoldEffect` / `HoverEffect`：优先 Host continuous session，失败时回退旧池
- `HoldEffect::OnCommand` 同时广播到 Host 与旧池，保证兼容

## 3. 兼容与回退

- 旧 `RippleWindowPool` 路径保留，不会一次性移除
- Host 不可用时（创建失败），行为退回历史实现
- 配置与 IPC 命令保持兼容

## 4. 验收点

1. 拖尾 + click/hover/hold 同时出现时，不再互相暂停
2. 连续滚轮触发不影响拖尾连续性
3. hold 百分比与命令更新（`hold_ms`/`threshold_ms`）正常
4. `reload_config` 后各效果仍可正常重建
