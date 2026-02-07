# OverlayHostWindow 合并计划

## 1. 背景与目标

当前特效是多窗口模型：

- `TrailWindow` / `ParticleTrailWindow`：全屏透明层，独立 16ms 定时器
- `RippleWindowPool`：点击/滚轮/长按/悬浮共享窗口类型，但每类独立窗口池
- `TextWindowPool`：文字点击特效独立窗口池
- `AppController`：统一分发输入消息（同一 UI 线程）

问题集中在：

1. 多窗口 TopMost 和 Z-Order 在外部窗口切换后容易不稳定
2. 多定时器并发时，某类效果可能表现为“暂停/卡顿”
3. 管理复杂，后续叠加新效果成本高

目标：逐步收敛为单一 `OverlayHostWindow`，由 Host 内部合成多个效果实例。

## 2. 设计边界

不改变的能力：

- 现有 JSON 指令与配置结构保持兼容
- 现有效果类型（trail/click/scroll/hold/hover/text）继续可用
- 渲染仍使用现有 GDI+ / D2D 资源体系，不引入重依赖

本次不做：

- 真 3D 引擎改造
- 全量替换现有 renderer 数学模型

## 3. 目标架构

引入一个宿主窗口 `OverlayHostWindow`：

- 单窗口全屏 layered + topmost + noactivate
- 单帧循环（统一 60fps Tick）
- 内部维护“效果实例列表”（点击脉冲、滚轮箭头、长按持续态、悬浮持续态、拖尾轨迹、文字浮动）
- 每帧按图层顺序合成并一次性 `UpdateLayeredWindow`

核心接口建议：

- `IOverlayLayer`：`Update(nowMs)` / `Render(GraphicsContext&)` / `IsAlive()`
- `OverlaySession`：用于持续态（hold/hover/trail）绑定输入更新
- `OverlayBurst`：用于瞬时态（click/scroll/text）自动回收

## 4. 分阶段迁移

### Phase 0: 基线固化（先做）

产出：

- 保持当前行为不变
- 记录窗口数量、定时器频率、CPU 占用、关键交互
- 增加 Host 开关配置（默认关闭）

验收：

- 当前版本行为与主分支一致
- 能通过配置切回旧实现

### Phase 1: 宿主框架落地（不迁移业务）

产出：

- 新增 `OverlayHostWindow`（创建、销毁、单 Tick、TopMost 保持）
- 新增 `OverlayCompositor`（空渲染）
- 增加运行时开关：`overlay_host.enabled`

验收：

- 开关关闭时，路径完全不变
- 开关开启时，Host 可正常运行但不接管效果

### Phase 2: 先迁移拖尾（风险最高先收敛）

产出：

- 把 `TrailWindow` 和 `ParticleTrailWindow` 的状态更新逻辑迁到 Host Layer
- 统一由 Host Tick 采样鼠标点并更新拖尾

验收：

- 拖尾不因点击/悬浮/长按暂停
- 顶层稳定，不再依赖低频扫描兜底
- 与旧版视觉误差可控（可接受微调）

### Phase 3: 迁移瞬时效果（click/scroll/text）

产出：

- `RippleWindowPool` 和 `TextWindowPool` 逻辑改为 Host 内部 burst 实例池
- 复用现有 renderer，避免重写数学逻辑

验收：

- 连点、连滚、文字连发不丢帧
- 不影响拖尾连续性

### Phase 4: 迁移持续效果（hold/hover）

产出：

- 持续态会话接入 Host（支持 `OnCommand`、`UpdatePosition`、`Stop`）
- 保持 hold 百分比和 hover 动画连续

验收：

- hold + trail + hover 同时存在时不互相暂停
- 结束态资源可正确回收

### Phase 5: 清理旧窗口实现

产出：

- 删除不再使用的窗口池与重复定时器路径
- 补齐文档与回归 checklist

验收：

- Release x64 编译通过
- 关键回归场景通过（见第 6 节）

## 5. 回滚策略

每个阶段都要求“可单独回滚”：

- 保留旧路径直到对应阶段稳定
- 通过配置开关快速切回旧实现
- 不做一次性大爆改

## 6. 回归矩阵（每阶段都执行）

1. 拖尾持续移动，同时点击/长按/悬浮，确认不暂停
2. 打开新顶层窗口（如 MAUI/设置页），确认特效仍在最上层
3. 多显示器移动，确认坐标和裁切正确
4. 高频滚轮 + 点击连发，确认无明显掉帧
5. `reload_config` 后行为一致

## 7. 当前执行顺序

当前建议先落地：

1. `Phase 1`：只上 Host 框架和开关
2. `Phase 2`：仅迁移 trail（line + particle）
3. trail 稳定后再推进 `Phase 3/4`

这样可以先解决你最关心的“拖尾层级和暂停”问题，再迁移其余效果。
