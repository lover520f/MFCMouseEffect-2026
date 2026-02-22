# Stage50 - Quantum Halo GPU Presenter 平台目录收敛

## 判定

`架构债务`：`QuantumHaloGpuV2Presenter` 与 `QuantumHaloGpuV2ShaderPipeline` 仍位于 `MouseFx` 目录，但实现本质上依赖 Win32 窗口消息、D3D11/DComp 与 DXGI，属于平台层能力，不应继续滞留在 core 路径。

## 目标

1. 将 Quantum Halo GPU Presenter/Shader Pipeline 的 Windows 实现迁移到 `Platform/windows`。
2. 保持上层接口与运行行为不变，避免回归。
3. 进一步压缩 core 目录中的 Win32 专有代码面，便于后续 macOS/Linux 实现对齐。

## 变更摘要

### 1) Windows 实现迁移到平台目录

迁移文件：

- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2Presenter.h/.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2ShaderPipeline.h/.cpp`

新路径：

- `MFCMouseEffect/Platform/windows/Renderers/Hold/QuantumHaloGpuV2Presenter.h/.cpp`
- `MFCMouseEffect/Platform/windows/Renderers/Hold/QuantumHaloGpuV2ShaderPipeline.h/.cpp`

### 2) 引用路径同步

调整引用：

- `MFCMouseEffect/Platform/windows/Renderers/Hold/QuantumHaloGpuV2Presenter.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.h`

目的：确保上层 backend 继续通过统一 presenter 接口工作，但具体实现路径落在平台目录。

### 3) 工程文件同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 移除旧 `MouseFx/Renderers/Hold` 下对应编译项。
- 增加 `Platform/windows/Renderers/Hold` 下的新编译项与头文件项。

## 验证

构建验证（Release|x64）：

1. 使用 VS2026 `MSBuild` 绝对路径构建 `MFCMouseEffect.vcxproj`。
2. 结果：`0 error / 0 warning`，链接成功生成 `x64/Release/MFCMouseEffect.exe`。

## 收益

1. 将 Windows GPU 渲染链路从 core 语义目录下沉到平台目录，职责边界更清晰。
2. Quantum Halo 路径与既有 Win32 平台工厂化方向保持一致，降低后续跨平台迁移阻力。
