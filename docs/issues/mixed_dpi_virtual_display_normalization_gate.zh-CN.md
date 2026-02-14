# 混合 DPI 多屏：Click/Trail 边界错位修复

## 问题现象
- 多显示器缩放比例不一致时（尤其副屏缩放低于主屏），副屏左右边界附近出现：
  - `Trail` 位置在正确/错误之间跳动
  - `Click` 持续偏移
- 同时 `Hold/Hover/Scroll` 基本正常。

## 根因
- 输入链路不一致：
  - `Hold/Hover/Scroll` 在分发阶段更依赖实时光标坐标（`GetCursorPos`）
  - `Click/Trail` 更依赖 hook 点（`MSLLHOOKSTRUCT.pt`）
- 在混合 DPI 边界区域，hook 点更容易产生抖动或偏移，导致两类效果表现分裂。

## 最终落地方案
文件：`MFCMouseEffect/MouseFx/Core/GlobalMouseHook.cpp`

- 统一取点策略为 **cursor-first**：
  - 优先 `GetCursorPos`
  - 失败时回退 hook 点
- 覆盖路径：
  - `WM_MOUSEMOVE`（Trail）
  - `WM_MFX_CLICK`（Click）
- `NormalizeScreenPoint(...)` 收敛为轻量包装，内部复用同一 cursor-first 逻辑，避免多套规则并存。

## 效果
- `Click/Trail` 与 `Hold/Hover/Scroll` 坐标链路保持一致。
- 混合 DPI 多屏边界区域错位与跳动显著收敛。
- 代码复杂度下降，去除不必要的环境探测分支。

## 验证建议
1. 主屏 150%、副屏 100%（或类似不对称缩放）下，在副屏左右边界附近快速移动并点击。
2. 观察 `Click`/`Trail` 是否紧贴真实光标，是否还出现跳动。
