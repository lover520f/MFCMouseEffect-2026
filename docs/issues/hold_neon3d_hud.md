# Hold Neon HUD (3D) - 长按概念图 HUD（GDI+ 分层复刻）

## 目标
尽量贴近 `docs/images/hold_concept.png`（以及你给的 HTML canvas demo v2）：
外圈玻璃壳 + 内壁微光扫描纹理 + 亮区方向性刻线 + 中央晶体种子 + “主干+分叉”的树状连接线 + 编织能量带 + 胶囊进度头 + 顶部百分比。

## 架构落点（为什么适配）
- `HoldEffect::OnHoldStart` 用 `ShowContinuous(..., params.loop=false)` 启动持续窗口。
- `RippleWindow` 在连续模式且不 loop 时会把 `t` 钳到 `1.0`，但窗口持续刷新：
  - `t` / `elapsedMs` 都可驱动进度与动画；满进度后 `elapsedMs` 继续增长，可保持“活着”的动效。

## 代码入口（核心文件）
- Renderer：`MFCMouseEffect/MouseFx/Renderers/Hold/HoldNeon3DRenderer.h`
- 分层绘制函数：
  - `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DRings.h`
  - `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h`
- 调色板（保证 cyan/purple/mint 互补）：`MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DColor.h`

更详细的“层级 ↔ 函数映射”见：`docs/issues/hold_neon3d/layers.md`。

## 进度与 0% 问题（已处理）
进度策略（renderer 内）：
1. 默认使用 `elapsedMs/thresholdMs`（窗口每帧都有 elapsedMs，因此**不移动鼠标也能持续充能**）。
2. 如果收到了上游的 `hold_ms`，只在首次到达时计算一次 **bias**（`bias = hold_ms - elapsedMs`），之后用 `elapsedMs + bias` 驱动进度：
   - 好处：既能对齐“更语义化的 hold_ms”，又不会因为 `hold_ms` 更新稀疏导致进度卡住/跳变。

同时 `AppController` 的 `WM_MFX_MOVE` 已补上 `holdMs` 计算，`OnHoldUpdate(pt, holdMs)` 不再恒为 0（但 renderer 仍保留回退逻辑）。

## “直径/链子”不动（已处理）
概念图箭头那束连接线不再用“固定直线”：
- 用 `DrawBranchTendrils(...)` 复刻 **主干+分叉** 的树状结构，末端吸附到亮区扇区并发光。
- 初始方向带随机偏置（`bundleBiasRad`），并叠加时间驱动摆动/抖动，使长按过程中不会“死板不动”。

## 与 HTML 参考的差异（必要适配）
- GDI+ 没有 canvas 的 `globalCompositeOperation='lighter'` 与 `shadowBlur`：
  - 用多次宽笔低 alpha 叠加来近似辉光。
- 叠加窗口尺寸通常是 `~220px`：
  - 百分比文本的角度 clamp 调整为更窄的顶部扇区，且 `outwardPx` 动态收缩，避免被窗口裁切。

## 进度弧“尾巴固定在 12 点”（概念图对齐）
为贴你标注的①（12 点锚点）：
- renderer 里把 **可读进度弧**（`DrawProgressMainArc` + `DrawProgressArcBase`）起点固定为 `-90°`（12 点方向），弧长随进度增长。
- 编织能量带也改为从 12 点铺到 `headAng`，只是在 head 附近更亮，避免出现“固定长度彩带在环上滑动”的观感。

## 配置与入口
- Renderer 名称：
  - 主名：`hold_neon3d`
  - 别名：`neon3d`

## 手工验证清单
1. Hold 选择 `Neon HUD (3D)`，长按触发。
2. 0~120ms：整体出现有轻微弹性缩放。
3. 充能阶段：百分比递增；亮区附近编织能量带流动；连接线末端有发光点与少量喷溅粒子。
4. 满进度后：百分比保持 100%，但能量带/连接线/粒子仍持续“活着”。

## 兼容性备注
项目当前编译选项未开启 `/std:c++17`，因此实现中避免使用 C++17 的嵌套命名空间写法（例如 `namespace a::b {}`），以保证在现有编译设置下可直接通过构建。
