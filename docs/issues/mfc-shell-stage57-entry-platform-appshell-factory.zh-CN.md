# Stage57 - 进程入口 AppShell 平台工厂化

## 判定

`架构债务`：`MFCMouseEffect.cpp` 仍直接包含并实例化 `Platform/windows/Shell/Win32AppShell`，入口层对 Windows 实现存在硬编码耦合，不利于后续多平台分包。

## 目标

1. 将进程入口从具体 `Win32AppShell` 解耦为平台工厂接口。
2. 保持现有 Windows 行为不变（命令行解析、托盘、消息循环、退出流程）。
3. 与既有 `Platform*Factory` 体系保持一致。

## 变更摘要

### 1) 新增平台 AppShell 抽象与工厂

新增：

- `MFCMouseEffect/Platform/IPlatformAppShell.h`
- `MFCMouseEffect/Platform/PlatformAppShellFactory.h`
- `MFCMouseEffect/Platform/PlatformAppShellFactory.cpp`

设计要点：

1. `IPlatformAppShell` 只暴露最小生命周期接口：
   - `Initialize()`
   - `RunMessageLoop()`
   - `Shutdown()`
2. Windows 下由 `Win32PlatformAppShell` 适配 `Win32AppShell`。
3. 非 Windows 预留 `NullPlatformAppShell`，为后续 macOS/Linux entry wiring 提供稳定接口。

### 2) 入口改为调用平台工厂

更新：

- `MFCMouseEffect/MFCMouseEffect.cpp`

变更：

1. 删除对 `Win32AppShell.h` 的直接 include。
2. 通过 `platform::CreatePlatformAppShell()` 获取抽象实例并驱动生命周期。

### 3) 兼容注释与工程清单同步

更新：

- `MFCMouseEffect/MFCMouseEffect.h`
- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

1. 入口注释改为指向 `Platform/PlatformAppShellFactory.h`。
2. 新增工厂与接口文件的编译/过滤器条目。

## 验证

1. `Release|x64` 构建通过：`0 error / 0 warning`。
2. 平台边界守卫脚本通过：
   - `tools/architecture/Check-NonPlatformWin32Boundary.ps1`
   - 输出：`OK: no direct Win32/MFC boundary violations in non-Platform compile units.`

## 收益

1. 进程入口彻底脱离 `Win32AppShell` 具体类型，平台切换点收敛到工厂。
2. 与 Flutter 式分平台装配思路更一致：入口稳定，平台实现可替换。
