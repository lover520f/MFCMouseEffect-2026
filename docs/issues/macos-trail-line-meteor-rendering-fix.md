# macOS 拖尾效果修复: 普通线条 + 绚丽流星

## 修复概述

| Bug | 文件 | 根因 | 修复 |
|-----|------|------|------|
| 普通线条拖尾无效果 | `MacosLineTrailOverlay.mm` | 路径不连续 + 统一alpha + fillColor非透明 | 逐段sublayer独立alpha |
| 绚丽流星有火柴棍 | `MacosTrailPulseOverlayRendererCore.Layers.mm` | 直线路径误用 | 方向性椭圆光斑 + 双层辉光 |

---

## Bug 1: 普通线条 (Line Trail)

### 根因

1. **悬垂引用 (`dispatch_async` + `const&`)**: `UpdateLineTrail(const ScreenPoint&, const LineTrailConfig&)` 将引用直接传入 `dispatch_async` block。ObjC++ block 捕获引用本身（不是值拷贝），调用方栈帧在 block 执行前已销毁 → 读到垃圾数据 → 只有最后一个点有效 → 仅显示单点。
2. 路径不连续（`CGPathMoveToPoint` 逐段）+ 统一 alpha + `fillColor` 非透明。
3. `durationMs ≈ 219ms` 太短 + idle fade 太激进。

### 修复方案

- `pathLayer`（`CAShapeLayer`）→ `containerLayer`（`CALayer`）作为容器
- 每条线段用独立 `CAShapeLayer` sublayer，独立设置 alpha（midpoint age-based）
- `fillColor` 始终 `clearColor`
- 头亮尾暗的渐弱效果
- `ClearSegmentSublayers()` 在每次重建前清除旧 sublayer
- `[CATransaction setDisableActions:YES]` 避免隐式动画

### 关键参数

- `alpha = pow(life, 0.6) * idleFactor`，min alpha 0.08（life > 0）
- `life = 1.0 - midAge / durationMs`
- 单点时绘制小圆点（dot），alpha 同样基于年龄

### 可见性修复

**问题**: 修复渲染后仍然效果极弱，原因是三个参数叠加:

| 参数 | 原值 | 修复后 | 原因 |
|------|------|--------|------|
| durationMs | ~219ms (0.73× 缩放) | ~548ms (2.5× 补偿, min 500) | pulse 缩放因子不适合连续线条 |
| idle fade start | 60ms | 300ms | 防止刚画完就开始淡出 |
| idle fade end | 220ms | 600ms | 给淡出更长的过渡期 |
| alpha 曲线 | `life * idleFactor` | `pow(life, 0.6)` + min 0.08 | 避免两个 <1 因子相乘导致近零 |
| lineWidth min | 1.0 | 2.0 | 确保最小可见宽度 |

---

## Bug 2: 绚丽流星 (Meteor Trail)

### 根因

`ConfigureTrailCoreLayer()` 中 meteor 走了通用 `else` 分支，用 `CreateTrailLinePath()` 创建一条直线路径，配合粗 `lineWidth` 呈现为火柴棍。

### 修复方案

新增 `trailType == "meteor"` 专用分支：

- **核心形状**: 沿运动方向拉伸的椭圆（`CGAffineTransform` 旋转）
  - `stretch = clamp(1.2 + speed * 0.04, 1.2, 2.8)`
  - `semiMajor = coreRadius * stretch`, `semiMinor = coreRadius`
- **填充模式**: `fillColor` = strokeArgb, `strokeColor` = clear, `lineWidth` = 0
- **双层辉光** (`AddTrailGlowLayer`):
  - Inner glow: 更大半径（inset 12→5 vs 18→8）
  - Outer glow: 暖色调 `#18FFDCA0` 外层光圈
