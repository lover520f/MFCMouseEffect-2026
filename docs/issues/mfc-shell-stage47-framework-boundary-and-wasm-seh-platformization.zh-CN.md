# Stage47 - framework 边界收敛与 Wasm SEH 平台化

## 背景

Windows 迁移到 Stage46 后，运行主链路仍有两处残留问题：

1. `pch.h` 仍通过根目录 `framework.h` 注入 Win32 头，平台边界不够清晰。
2. `WasmRuntimeBridgeContext.cpp` 直接包含 `windows.h`，同时内嵌 `__try/__except` 逻辑，导致上下文文件承担了平台细节。

## 本阶段目标

1. 让运行主链路不再依赖 `framework.h`。
2. 将 Wasm 运行时 SEH 保护迁移到平台实现文件，保持 `RuntimeBridgeContext` 聚焦业务流程。

## 变更摘要

### 1) 新增平台基础头并切换 PCH 入口

新增：

- `MFCMouseEffect/Platform/windows/Common/Win32Base.h`

调整：

- `MFCMouseEffect/pch.h`
  - 从 `#include "framework.h"` 改为 `#include "Platform/windows/Common/Win32Base.h"`。
- `MFCMouseEffect/framework.h`
  - 改为历史兼容壳，仅转发到 `Win32Base.h`，避免存量代码断裂。

### 2) 清理运行链路对 framework 的直接依赖

移除以下文件中的 `#include "framework.h"`：

- `MFCMouseEffect/Platform/windows/Shell/Win32EventLoopService.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Win32DpiAwarenessService.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Win32SingleInstanceGuard.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Win32SettingsLauncher.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Win32UserNotificationService.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Tray/Win32TrayHostWindow.cpp`

### 3) WasmRuntimeBridge 的 SEH 平台化拆分

新增：

- `MFCMouseEffect/WasmRuntimeBridge/Platform/windows/WasmRuntimeSehGuard.h`
- `MFCMouseEffect/WasmRuntimeBridge/Platform/windows/WasmRuntimeSehGuard.cpp`

调整：

- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`
  - 删除 `windows.h`/`excpt.h` 直接包含。
  - 删除内嵌 `SafeM3Call/SafeM3GetResults` 的 `__try/__except` 实现。
  - 改为调用 `WasmRuntimeSehGuard`。
  - SEH code 类型从 `DWORD` 收敛为 `uint32_t`（消息文本逻辑保持不变）。

### 4) 工程文件同步

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - 增加 `Platform\windows\Common\Win32Base.h`。
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`
  - 增加 `Win32Base.h` 过滤器映射。
- `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj`
  - 增加 `WasmRuntimeSehGuard.h/.cpp`。
- `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj.filters`
  - 增加 `WasmRuntimeSehGuard.h/.cpp` 过滤器映射。

## 验证

使用本机 VS 2026 Professional `MSBuild`（x64）：

1. `mfx_wasm_runtime.vcxproj` Release|x64 编译通过，0 error。
2. `MFCMouseEffect.vcxproj` Release|x64 编译通过，0 error。

说明：

- 一次并行验证中出现过 `C1041`（同一 `pdb` 并行争用），由同时启动两个 MSBuild 实例引起；改为串行后稳定通过，不属于代码逻辑回归。

## 收益

1. 运行主链路的 Win32 头入口完成平台路径收敛（`Platform/windows/Common`）。
2. `RuntimeBridgeContext` 的职责更单一，SEH 平台细节剥离完成，后续做非 Windows RuntimeBridge 适配时改动面更小。
3. 为后续“彻底清理历史 `framework.h`/存量 UI 源”提供过渡兼容层，降低一次性重构风险。
