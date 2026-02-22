# MFC 壳解耦阶段 36：WASM 动态桥运行时平台工厂化

## 1. 背景与目标

阶段 35 后，WASM 动态桥运行时仍位于 Core 层：

- `MFCMouseEffect/MouseFx/Core/Wasm/DllWasmRuntime.h/.cpp`

该实现直接依赖 `windows.h`、`LoadLibraryW/GetProcAddress`，属于平台实现，不应由 Core 直接持有。

本阶段目标：

- 将 Win32 动态桥运行时迁移到 `Platform/windows/Wasm`；
- 引入平台工厂入口，避免 Core 直接 `new` Win32 实现；
- 保持 WASM 功能行为与降级逻辑不变。

## 2. 判定

判定：`架构演进`（平台边界收口，不改功能行为）。

依据：

- 动态桥初始化、导出符号解析、调用与错误回退逻辑保持一致；
- Core 仅切换为调用 `Platform` 工厂。

## 3. 实施内容

### 3.1 Win32 运行时实现迁移

从：

- `MFCMouseEffect/MouseFx/Core/Wasm/DllWasmRuntime.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/Wasm/Win32DllWasmRuntime.h/.cpp`

实现类重命名：

- `DllWasmRuntime` -> `Win32DllWasmRuntime`

说明：

- 接口仍是 `IWasmRuntime`；
- 导出函数解析和桥接调用保持不变。

### 3.2 新增平台工厂

新增文件：

- `MFCMouseEffect/Platform/PlatformWasmRuntimeFactory.h`
- `MFCMouseEffect/Platform/PlatformWasmRuntimeFactory.cpp`

新增 API：

- `mousefx::platform::CreateDynamicBridgeWasmRuntime(std::string* outError)`

职责：

- 平台层创建并初始化 `Win32DllWasmRuntime`；
- 失败时返回 `nullptr` 和错误信息，供 Core 统一回退。

### 3.3 Core 工厂改为平台注入

文件：

- `MFCMouseEffect/MouseFx/Core/Wasm/WasmRuntimeFactory.cpp`

变更：

- 移除对 `DllWasmRuntime.h` 的直接依赖；
- 改为调用 `PlatformWasmRuntimeFactory` 获取动态桥运行时；
- 保留原有回退到 `NullWasmRuntime` 的策略和诊断文本。

### 3.4 工程文件同步

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 移除 `MouseFx/Core/Wasm/DllWasmRuntime.*`；
- 加入 `Platform/windows/Wasm/Win32DllWasmRuntime.*`；
- 加入 `Platform/PlatformWasmRuntimeFactory.*`；
- filters 映射到 `头文件\Platform` / `源文件\Platform`。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与边界

已控制：

- Core 回退策略保持一致，动态桥不可用时仍使用 `NullWasmRuntime`；
- `git mv` 保留迁移历史。

当前剩余（Windows 跨平台收口）：

- 仍有部分 Win32 依赖在非 `Platform/windows`（如 `GpuProbeHelper`、部分工具/渲染头），后续阶段继续收敛。
