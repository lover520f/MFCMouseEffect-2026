# Stage56 - Hold Renderer Linkage 拆分与 Win32 GPU Renderer 下沉

## 判定

`边界泄漏`：`RippleBasedHoldRuntime.cpp` 通过 include 多个 hold renderer 头触发静态注册，其中包含 Windows GPU renderer，导致 core runtime 文件直接依赖平台实现路径。

## 目标

1. 将 hold renderer 静态注册从 runtime 逻辑文件中拆出。
2. 将两个 Windows GPU hold renderer 头下沉到 `Platform/windows`。
3. 保持渲染器注册行为不变，并修复迁移后头文件自包含问题。

## 变更摘要

### 1) 拆分 hold renderer 注册入口

新增：

- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldRendererLinkage.cpp`
- `MFCMouseEffect/Platform/windows/Renderers/Hold/Win32HoldRendererLinkage.cpp`

作用：

- `HoldRendererLinkage.cpp` 负责平台中立 hold renderer 的静态注册。
- `Win32HoldRendererLinkage.cpp` 负责 Windows GPU hold renderer 的静态注册。

### 2) 移除 RippleBasedHoldRuntime 的注册耦合

更新：

- `MFCMouseEffect/MouseFx/Effects/RippleBasedHoldRuntime.cpp`

变更：

- 删除“为触发静态注册而 include renderer 头”的代码块。
- runtime 仅保留运行时逻辑，不再承担注册装配职责。

### 3) Windows GPU hold renderer 头下沉

迁移：

- `MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`
  -> `Platform/windows/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`
- `MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
  -> `Platform/windows/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`

并修正：

- `HoldQuantumHaloGpuV2Renderer.h` 的 `QuantumHaloPresenterHost` 引用路径改为 core 全路径。
- 两个头都显式包含 `MouseFx/Renderers/RendererRegistry.h`，确保自包含。

### 4) 工程同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 添加 `HoldRendererLinkage.cpp` 与 `Win32HoldRendererLinkage.cpp` 编译项。
- filters 中同步归档到 `源文件\MouseFx` 与 `源文件\Platform`。

## 验证

1. `Release|x64` 构建通过：`0 error / 0 warning`。
2. 边界守卫脚本通过：
   - `tools/architecture/Check-NonPlatformWin32Boundary.ps1`
   - 输出 `OK: no direct Win32/MFC boundary violations in non-Platform compile units.`

## 收益

1. `RippleBasedHoldRuntime` 从“逻辑 + 注册装配”混合职责中解耦，单一职责更清晰。
2. Windows GPU hold renderer 正式归位平台目录，core 侧平台耦合继续下降。
