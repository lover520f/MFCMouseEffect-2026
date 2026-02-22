# Stage53 - Quantum Halo DComp Backend 平台化下沉

## 判定

`架构债务`：`QuantumHaloDCompPresenterBackend` 虽然是 Windows DComp/D3D11 路由后端，但文件位置仍在 `MouseFx/Renderers/Hold/Presentation`，导致 core presentation 目录混入平台实现。

## 目标

1. 将 `QuantumHaloDCompPresenterBackend` 下沉到 `Platform/windows`。
2. 保持 `IQuantumHaloPresenterBackend`、host、registry 等平台中立骨架继续留在 `MouseFx`。
3. 不改变现有注册机制和运行时选择逻辑。

## 变更摘要

### 1) 文件迁移

从：

- `MFCMouseEffect/MouseFx/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.h/.cpp`

### 2) 头文件依赖修正

更新：

- `MFCMouseEffect/Platform/windows/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.h`
- `MFCMouseEffect/Platform/windows/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.cpp`

说明：

- backend 通过全路径引用 core 抽象与 registry：
  - `MouseFx/Renderers/Hold/Presentation/IQuantumHaloPresenterBackend.h`
  - `MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterBackendRegistry.h`

### 3) 工程同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 将 `QuantumHaloDCompPresenterBackend` 编译项和头文件项切换为 `Platform/windows/...`。
- filters 归类从 `MouseFx` 调整到 `Platform`。

## 验证

1. `Release|x64` 构建通过：`0 error / 0 warning`。
2. `tlog` 清理显示旧 `MouseFx/Renderers/Hold/Presentation/QuantumHaloDCompPresenterBackend.cpp` 已从构建项移除。

## 收益

1. Quantum Halo presentation 层边界更清晰：core 保留抽象与调度，Windows 后端落在平台目录。
2. 后续新增非 Windows presenter backend 时，可按同模式接入而不污染 `MouseFx` 核心目录。
