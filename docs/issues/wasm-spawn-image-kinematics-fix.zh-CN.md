# WASM spawn_image 运动轨迹修复

## 现象

WASM 图片特效看起来只是“在点击点闪一下”：
- 图片能显示，
- 但几乎没有轨迹位移，
- 用户体感是“图片特效没生效”。

## 根因

问题由两层叠加导致：

1. 宿主执行链路未真正应用 `spawn_image` 的运动语义。
- `SpawnImageCommandV1` 有 `vx/vy/ax/ay/delayMs` 字段；
- 旧链路只传了方向和强度；
- `RippleOverlayLayer` 一直按点击点固定中心渲染。

2. 模板里多组图片样例参数接近“位移抵消”。
- `vy` 较小且 `ay` 偏大，生命周期内净位移很小。

## 改动

## 1. 宿主补齐运动与延迟语义

在 `RenderParams` 与 Overlay 层增加可选运动模型：
- `useKinematics`
- `velocityX/velocityY`
- `accelerationX/accelerationY`
- `startDelayMs`

修改文件：
- `MFCMouseEffect/MouseFx/Interfaces/IRippleRenderer.h`
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h`
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`

行为：
- `useKinematics=true` 时，中心点按
  `dx = vx*t + 0.5*ax*t^2`、`dy = vy*t + 0.5*ay*t^2` 更新；
- `delayMs` 会延迟显现，不会直接丢弃实例；
- 非 WASM 既有效果默认不受影响（运动默认关闭）。

## 2. 模板样例轨迹重调

把图片相关样例参数调成“肉眼可见位移”：
- `image-pulse.ts`
- `image-burst.ts`
- `image-lift.ts`
- `mixed-text-image.ts`
- `mixed-emoji-celebrate.ts`
- `button-adaptive.ts`

## 验证

- 模板：`pnpm run build:samples` 全部样例通过；
- 工程：`MSBuild x64 Debug` 0 error 通过。

## 结果

`spawn_image` 现在已具备真实特效表现：
- 有明显运动轨迹；
- 延迟语义生效；
- 默认样例视觉反馈明显提升。
