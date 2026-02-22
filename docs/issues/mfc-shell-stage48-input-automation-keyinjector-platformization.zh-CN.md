# Stage48 - 输入自动化键盘注入平台化

## 判定

`Bug或架构债务`：`InputAutomationEngine` 之前直接依赖 `Core/Automation/KeyboardInjector`，该实现内部调用 Win32 `SendInput`，导致 Core 自动化层仍有平台实现耦合，不利于跨平台迁移。

## 目标

1. Core 自动化层仅依赖抽象接口，不直接绑定 Win32 注入实现。
2. Windows 注入逻辑下沉到 `Platform/windows/System`。
3. 保持现有自动化行为和配置协议不变。

## 变更摘要

### 1) 新增键盘注入抽象与空实现

新增：

- `MFCMouseEffect/MouseFx/Core/System/IKeyboardInjector.h`
- `MFCMouseEffect/MouseFx/Core/System/NullKeyboardInjector.h`

说明：

- `IKeyboardInjector::SendChord` 作为统一注入能力接口。
- 非 Windows 或服务不可用时由 `NullKeyboardInjector` 返回 `false`，避免 Core 层分支散落。

### 2) Win32 注入实现迁移到平台层

新增：

- `MFCMouseEffect/Platform/windows/System/Win32KeyboardInjector.h`
- `MFCMouseEffect/Platform/windows/System/Win32KeyboardInjector.cpp`

实现要点：

- 保留原有 `KeyChord` 解析与按键下发语义。
- 使用 `SendInput + MapVirtualKeyW`，行为与旧 `KeyboardInjector` 对齐。

### 3) InputAutomationEngine 改为接口注入

调整：

- `MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.h`
- `MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.cpp`

关键点：

- 移除 `KeyboardInjector` 成员，改为 `IKeyboardInjector* keyboardInjector_`。
- 新增 `SetKeyboardInjector(IKeyboardInjector*)`。
- `TriggerMouseAction/TriggerGesture` 在注入器为空时直接返回 `false`。

### 4) AppController 与平台工厂接线

调整：

- `MFCMouseEffect/Platform/PlatformInputServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformInputServicesFactory.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键点：

- 新增 `platform::CreateKeyboardInjector()`。
- Windows 返回 `Win32KeyboardInjector`，其他平台返回 `NullKeyboardInjector`。
- `AppController` 持有 `std::unique_ptr<IKeyboardInjector>`，并注入到 `InputAutomationEngine`。

### 5) 删除旧实现并同步工程

删除：

- `MFCMouseEffect/MouseFx/Core/Automation/KeyboardInjector.h`
- `MFCMouseEffect/MouseFx/Core/Automation/KeyboardInjector.cpp`

工程同步：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

已移除旧 `KeyboardInjector` 条目，新增接口/空实现/Win32 实现条目。

## 验证

构建验证（Release|x64）：

1. `MFCMouseEffect.vcxproj` 编译通过，0 warning，0 error。
2. 首次出现 `LNK1104`（`MFCMouseEffect.exe` 占用）；自动结束占用进程后重试通过。

回归关注点：

1. 自动化鼠标映射触发快捷键（单键与组合键）行为保持一致。
2. 手势映射链路触发快捷键行为保持一致。

## 收益

1. Core 自动化从 Win32 具体实现解耦，平台职责边界更清晰。
2. 后续接入 macOS/Linux 键盘注入时，只需新增平台实现并挂接工厂，不需改 `InputAutomationEngine` 业务逻辑。
