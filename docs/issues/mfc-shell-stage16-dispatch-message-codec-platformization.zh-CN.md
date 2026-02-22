# MFC 壳解耦阶段 16：Dispatch Message Codec 平台化（Win32 首落地）

## 1. 背景与目标

阶段 15 已把 Dispatch Message Host（消息窗口宿主）平台化，但 `DispatchRouter` 仍直接消费 Win32 消息语义（`WM_*`、`WPARAM/LPARAM`、`DefWindowProcW`），Core 层还存在平台协议泄漏。

本阶段目标：

- 引入平台无关的 `DispatchMessage` 结构；
- 新增消息编解码接口，把 Win32 消息解析下沉到 `Platform/windows`；
- `DispatchRouter` 仅处理规范化消息，不再依赖 Win32 消息宏与消息常量；
- 保持未知消息与未知定时器的默认处理行为（回落平台默认结果）。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 用户可见行为不变（点击/移动/滚轮/按键/长按/悬停效果链路保持一致）；
- 主要变更为消息协议边界收敛与平台职责下沉。

## 3. 实施内容

### 3.1 Core 新增消息模型与编解码抽象

新增：

- `MFCMouseEffect/MouseFx/Core/Control/DispatchMessage.h`
- `MFCMouseEffect/MouseFx/Core/Control/IDispatchMessageCodec.h`
- `MFCMouseEffect/MouseFx/Core/Control/NullDispatchMessageCodec.h`

设计要点：

- `DispatchMessageKind` 统一表达 `Click/Move/Scroll/Key/ButtonDown/ButtonUp/Timer/ExecCmd/GetConfig/Unknown`；
- `DispatchMessage` 承载平台无关字段与 native 原始字段（用于活动统计和默认回落）；
- `NullDispatchMessageCodec` 提供非 Win32 平台安全退化。

### 3.2 新增平台消息编解码工厂

新增：

- `MFCMouseEffect/Platform/PlatformControlMessageCodecFactory.h`
- `MFCMouseEffect/Platform/PlatformControlMessageCodecFactory.cpp`

策略：

- Windows 返回 `Win32DispatchMessageCodec`；
- 非 Windows 返回 `NullDispatchMessageCodec`。

### 3.3 Win32 消息编解码实现下沉

新增：

- `MFCMouseEffect/Platform/windows/Control/Win32DispatchMessageCodec.h`
- `MFCMouseEffect/Platform/windows/Control/Win32DispatchMessageCodec.cpp`

职责：

- 把 `WM_MFX_*` 与 `WM_TIMER` 映射到 `DispatchMessageKind`；
- 解析 `wParam/lParam` 到标准字段；
- 提供 `DefaultResult`（Win32 下走 `DefWindowProcW`）。

### 3.4 AppController 接入 codec 抽象

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键变更：

- 新增成员 `std::unique_ptr<IDispatchMessageCodec> dispatchMessageCodec_`；
- 构造时通过平台工厂注入，失败时回退 `NullDispatchMessageCodec`；
- `OnDispatchMessage(...)` 改为：
  1. 先 `Decode(...)`，
  2. 再 `DispatchRouter::Route(...)`，
  3. 未处理时回落 `codec->DefaultResult(...)`。

附带整理：

- 新增 `DisarmHoldTimer()`，由 Router 通过 Host 抽象回收 Hold 定时器。

### 3.5 DispatchRouter 改为消费规范化消息

修改：

- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.h`
- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

关键变更：

- 路由入口改为 `Route(const DispatchMessage&, bool* outHandled)`；
- 各分支处理函数统一改为消费 `DispatchMessage`；
- 移除 Core 内对 `WM_*` 路由判定与 `GET_X_LPARAM/GET_Y_LPARAM` 解析依赖；
- 对未知消息/未知 timer 通过 `outHandled=false` 回落平台默认处理，避免行为退化。

### 3.6 工程文件同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增编译项：

- `Platform\PlatformControlMessageCodecFactory.cpp`
- `Platform\windows\Control\Win32DispatchMessageCodec.cpp`

新增头文件项：

- `DispatchMessage`
- `IDispatchMessageCodec`
- `NullDispatchMessageCodec`
- `PlatformControlMessageCodecFactory`
- `Win32DispatchMessageCodec`

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

说明：

- Release 过程中如出现 `LNK1104`（目标 exe 被占用），按协作约定已先自动结束进程再重试构建。

## 5. 风险与边界

已控制：

- 未改变效果算法与触发条件，仅调整消息协议与平台边界；
- 未知消息与未知 timer 保留默认处理回落，避免消息链路行为变化。

当前边界：

- 输入来源仍由 Win32 Hook/窗口消息驱动，非 Windows 仅完成抽象与 Null 退化；
- `DispatchMessage` 仍携带 native 字段用于过渡期观测，后续可继续瘦身。

## 6. 后续建议（阶段 17）

- 抽象平台输入事件注入层，逐步减少 `Core/Protocol/InputTypesWin32.h` 在控制路径中的可见性；
- 将剩余 `MouseFx/Windows` 目录中的 Win32 具体能力按模块迁移到 `Platform/windows`；
- 为 macOS/Linux 增加最小输入事件桥实现（先保证事件闭环，再补完整能力）。
