# Stage 47 - FluxField 菜单文案对齐（去掉“全GPU”后缀）

## 问题判定
- 结论：`设计文案不一致`，不是功能 Bug。
- 依据：同一组长按菜单中，`量子光环 GPU` 与 `磁通场 HUD GPU（全GPU）` 命名风格不统一。

## 修改内容
- 文件：`MFCMouseEffect/Settings/SettingsOptions.h`
  - `磁通场 HUD GPU（全GPU）` -> `磁通场 HUD GPU`
  - `FluxField HUD GPU (Full GPU)` -> `FluxField HUD GPU`
- 文件：`MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
  - 同步上述中英文回退文案。

## 影响范围
- 仅 UI/托盘显示文案变化。
- 不改变渲染链路，不改变 GPU/CPU 路由策略。

## 验证步骤
1. 打开设置页长按特效下拉。
2. 打开托盘菜单长按特效子菜单。
3. 预期：均显示为 `磁通场 HUD GPU` / `FluxField HUD GPU`，不再带 `全GPU/Full GPU` 后缀。
