# 霓虹HUD长按首触发预热（Stage 68）

## 问题

启动/切换卡顿已显著下降，但当长按特效为“霓虹HUD（neon3d）”时，首次触发仍有明显卡顿。

## 调整

在 `HoldEffect::Initialize()` 增加重型长按渲染器预热：

1. 识别重型类型
- `neon3d`
- `hold_neon3d`
- `hologram_hud`

2. 启动阶段离屏预热
- 创建渲染器实例
- 构造低成本、零透明可见度的预热样式
- 在内存位图上执行两帧 `Render`（含一次 hold 进度/阈值更新）

## 目的

将首次渲染器冷启动开销前移到初始化阶段，减少首次长按交互时的突发卡顿。

## 变更文件

- `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
