# 拖动窗口场景输入节流（Stage 67）

## 问题

切到 Dawn 后，在“拖动应用窗口”期间仍出现短时卡顿。普通移动稳定后正常。

## 调整

在 `AppController` 的 `WM_MFX_MOVE` 分发层新增“拖动窗口场景节流”：

1. 识别拖动场景
- 条件：按键按下 + 系统鼠标捕获到非派发窗口（`GetCapture() != nullptr && != dispatchHwnd`）

2. 拖动场景降频
- 对 `Trail` / `Hold` 的 move 更新做节流：`20ms` 间隔（约 50 FPS）
- 非拖动场景保持原行为，不降频

## 目的

减少窗口拖动期间高频 move 事件导致的 CPU 抢占与渲染抖动，降低卡顿体感。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/AppController.h`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
