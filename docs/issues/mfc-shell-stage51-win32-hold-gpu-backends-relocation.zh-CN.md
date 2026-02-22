# Stage51 - Hold GPU 计算与 D2D 后端平台化下沉

## 判定

`架构债务`：`FluxFieldGpuV2ComputeEngine`、`QuantumHaloGpuV2ComputeEngine` 以及两个 `*GpuV2D2DBackend` 仍位于 `MouseFx/Renderers/Hold`，但其实现完全绑定 D3D11/D2D/Win32 运行时，属于 Windows 平台实现，不应继续放在 core 渲染目录。

## 目标

1. 将上述 4 组 Windows 专有 GPU 文件迁移到 `Platform/windows/Renderers/Hold`。
2. 保持上层 renderer 与 runtime 的行为不变，只调整依赖边界。
3. 同步工程和过滤器配置，保证构建与 IDE 结构一致。

## 变更摘要

### 1) 文件迁移

从：

- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldGpuV2ComputeEngine.h/.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2ComputeEngine.h/.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h/.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2D2DBackend.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/Renderers/Hold/FluxFieldGpuV2ComputeEngine.h/.cpp`
- `MFCMouseEffect/Platform/windows/Renderers/Hold/QuantumHaloGpuV2ComputeEngine.h/.cpp`
- `MFCMouseEffect/Platform/windows/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h/.cpp`
- `MFCMouseEffect/Platform/windows/Renderers/Hold/HoldQuantumHaloGpuV2D2DBackend.h/.cpp`

### 2) 引用同步

更新引用文件：

- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`
- `MFCMouseEffect/Platform/windows/Effects/Win32HoldQuantumHaloGpuV2DirectRuntime.cpp`

统一改为包含 `Platform/windows/Renderers/Hold/*` 路径。

### 3) 工程同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 4 个编译项从 `MouseFx/Renderers/Hold` 切换到 `Platform/windows/Renderers/Hold`。
- 新增 4 个平台头文件项和对应 filters 条目。

## 验证

构建验证（Release|x64）：

1. 使用 VS2026 `MSBuild` 绝对路径构建 `MFCMouseEffect.vcxproj`。
2. 结果：`0 error / 0 warning`，链接成功生成 `x64/Release/MFCMouseEffect.exe`。
3. tlog 清理显示旧 `MouseFx/Renderers/Hold/QuantumHaloGpuV2ComputeEngine.cpp` 已被移除，迁移生效。

## 收益

1. Hold 路径中 Windows 专有 GPU 计算与 D2D 可视化后端已集中到平台层。
2. `MouseFx` 目录进一步收敛为平台中立逻辑，有利于后续 macOS/Linux 平台实现替换。
