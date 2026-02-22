# Stage49 - Hold Direct Runtime 平台工厂化

## 判定

`架构债务`：`HoldEffect` 之前直接包含 `MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.h`，该 runtime 内部依赖 Win32 事件、消息泵与 COM 初始化，导致 core effect 层直接耦合 Windows 实现。

## 目标

1. 将 hold direct runtime 的创建入口从 core effect 层下沉到平台工厂。
2. Windows 实现放入 `Platform/windows`，core 仅依赖抽象工厂接口。
3. 不改变现有 hold 效果行为与路由策略。

## 变更摘要

### 1) 新增平台 hold runtime 工厂

新增：

- `MFCMouseEffect/Platform/PlatformHoldRuntimeFactory.h`
- `MFCMouseEffect/Platform/PlatformHoldRuntimeFactory.cpp`

接口：

- `std::unique_ptr<IHoldRuntime> CreatePlatformHoldRuntime(const std::string& type);`

当前策略：

- `type` 命中 `quantum_halo_gpu_v2_direct` 路由时，Windows 下返回平台实现。
- 非 Windows 返回 `nullptr`，由上层继续走 Ripple 路径。

### 2) Windows direct runtime 下沉

新增：

- `MFCMouseEffect/Platform/windows/Effects/Win32HoldQuantumHaloGpuV2DirectRuntime.h`
- `MFCMouseEffect/Platform/windows/Effects/Win32HoldQuantumHaloGpuV2DirectRuntime.cpp`

迁移内容：

- 原 `HoldQuantumHaloGpuV2DirectRuntime` 的 Win32 事件、线程、消息泵、COM 生命周期逻辑完整迁移。
- 类名调整为 `Win32HoldQuantumHaloGpuV2DirectRuntime`，语义更明确。

### 3) HoldEffect 改为通过平台工厂请求 direct runtime

调整：

- `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`

关键点：

- 移除对 `HoldQuantumHaloGpuV2DirectRuntime` 的直接 include。
- 在 direct 路由分支调用 `platform::CreatePlatformHoldRuntime(type)`。
- 若平台工厂返回空，保持既有 fallback（Ripple-based runtime）。

### 4) 删除旧路径实现并同步工程

删除：

- `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.h`
- `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.cpp`

工程同步：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增了 `PlatformHoldRuntimeFactory` 与 `Win32HoldQuantumHaloGpuV2DirectRuntime` 的编译/头文件项。

## 验证

构建验证（Release|x64）：

1. `MFCMouseEffect.vcxproj` 编译通过，0 warning，0 error。
2. tlog 清理显示旧 `HoldQuantumHaloGpuV2DirectRuntime.cpp` 已从构建项移除，迁移生效。

## 收益

1. `HoldEffect` 从具体 Win32 runtime 解耦，保持核心 effect 逻辑平台中立。
2. 未来 macOS/Linux 若要实现 direct hold runtime，只需扩展平台工厂与平台实现，不需改 core 路由逻辑。
