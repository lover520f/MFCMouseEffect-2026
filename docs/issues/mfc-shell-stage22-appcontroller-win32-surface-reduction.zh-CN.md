# MFC 壳解耦阶段 22：AppController Win32 类型暴露收敛

## 1. 背景与目标

阶段 21 已完成前台进程服务平台化，但 `AppController` 头文件仍直接暴露 Win32 类型（`HWND/WPARAM/DWORD/UINT_PTR`）并间接依赖 `windows.h`。

这会导致：

- Core 控制层接口继续被 Windows 基础类型污染；
- 后续 macOS/Linux 复用 `AppController` 时，编译边界不够干净。

本阶段目标：

- 将 `AppController` 对外接口统一到平台无关基础类型；
- 消除 `DispatchRouter` 对 `HWND + KillTimer` 的直接调用；
- 保持行为不变，仅做接口面收敛。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 点击/移动/滚动/长按路由逻辑未变；
- 仅替换类型与调用边界，不改变效果行为和配置语义。

## 3. 实施内容

### 3.1 AppController 对外接口去 Win32 类型

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键变更：

- `OnDispatchActivity(UINT, WPARAM)` -> `OnDispatchActivity(uint32_t, uintptr_t)`
- `CurrentHoldDurationMs()` 返回值 `DWORD` -> `uint32_t`
- `HoverTimerId/HoldTimerId` 类型 `UINT_PTR` -> `uintptr_t`
- `HoldDelayMs` 类型 `DWORD` -> `uint32_t`
- `StartDiagnostics.error` 类型 `DWORD` -> `uint32_t`

### 3.2 移除 AppController 对 HWND 的公开暴露

变更：

- 删除 `DispatchWindowHandle()` 对外方法；
- 新增 `KillDispatchTimer(uintptr_t timerId)`，由 `AppController` 内部转发给 `IDispatchMessageHost`。

收益：

- 调度路由不再依赖 native window handle；
- timer 生命周期统一通过平台抽象层接口管理。

### 3.3 DispatchRouter 去掉 Win32 KillTimer 直连

修改：

- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

关键变更：

- 自测路径不再调用 `KillTimer(hwnd, ...)`；
- 改为 `ctrl_->KillDispatchTimer(kSelfTestTimerId)`；
- timer id 统一用 `uintptr_t`。

### 3.4 进一步减少头文件传递耦合

修改：

- `AppController` 内部对象：
  - `GdiPlusSession` 改为 `std::unique_ptr<GdiPlusSession>`
  - `VmForegroundDetector` 改为 `std::unique_ptr<VmForegroundDetector>`
- 在头文件使用前置声明，具体实现留在 `.cpp`。

效果：

- `AppController.h` 不再直接写入 Win32 基础类型接口；
- 为后续将 `VmForegroundDetector` / GDI+ 初始化进一步平台化留下空间。

## 4. 验证

构建验证（VS 2026 MSBuild，amd64 路径）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- timer 行为不变，仍通过 `IDispatchMessageHost` 控制；
- `CurrentHoldDurationMs` 数值范围保持 `0..0xFFFFFFFF`，仅类型名替换。

当前边界：

- `Core` 其他模块仍有 Win32 依赖（例如 `VmForegroundDetector.h`、`OverlayCoordSpace.h`、`KeyChord` 等）；
- 本阶段仅收敛 `AppController/DispatchRouter` 这条控制链路表面耦合。

## 6. 后续建议（阶段 23）

- 抽象 `VmForegroundDetector` 为平台服务并迁移到 `Platform/windows/System`；
- 继续清理 `Core/Overlay` 与 `Core/Automation` 中残留的 Win32 头文件直连；
- 保持“Core 只依赖接口，Platform 提供实现”的统一规则。
