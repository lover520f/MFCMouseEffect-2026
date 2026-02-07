# OverlayHost Phase 4: Text 迁移

## 1. 目标

把 `TextEffect` 迁移到 `OverlayHostWindow`，继续收敛到单一画板：

- Host 优先渲染
- Host 不可用时回退 `TextWindowPool`

## 2. 代码改动

### 2.1 新增 Text 图层

- `MouseFx/Layers/TextOverlayLayer.h/.cpp`

职责：

- 管理文字实例生命周期（出现、漂移、淡出）
- 在 Host 统一帧循环中更新与渲染

实现要点：

- 复刻原文字运动参数：`driftX/swayFreq/swayAmp`、`EaseOutCubic`、缩放与淡出
- 支持字体回退与 emoji-only 文本字体选择

### 2.2 扩展 Host 服务

- `MouseFx/Core/OverlayHostService.h/.cpp`

新增接口：

- `ShowText(const POINT&, const std::wstring&, Argb, const TextConfig&)`

服务内部按需创建 `TextOverlayLayer`，并复用已有 Host 生命周期。

### 2.3 接入 TextEffect

- `MouseFx/Effects/TextEffect.cpp`

策略：

- 点击时优先调用 `OverlayHostService::ShowText`
- 若失败，动态初始化并走 `TextWindowPool` 回退路径

## 3. 兼容说明

- JSON 配置与文本内容接口保持不变
- click = `text` 时默认进入 Host 路径
- Host 初始化失败时行为与历史版本一致

## 4. 验收点

1. 点击 text 时文字可正常出现、漂移、淡出
2. 与 trail/ripple/hold/hover 同时触发时，不互相暂停
3. `reload_config` 后 text 仍可正常重建
4. Host 不可用时，能自动回退旧 `TextWindowPool`

## 5. 兼容修正

本阶段后续修正：

- emoji 文本检测到后，优先走旧 `TextWindowPool`（D2D 路径），保证表情可用
- Host 文本字号改为与旧实现一致的 point 语义，修复“字体偏大”问题
