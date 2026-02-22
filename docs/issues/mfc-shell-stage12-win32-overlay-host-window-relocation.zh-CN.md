# MFC 壳解耦阶段 12：Overlay Host Window 物理下沉到 Platform/windows

## 1. 背景与目标

阶段 11 已完成 `OverlayHostService -> IOverlayHostBackend` 抽象，但 Win32 具体窗口实现仍在 `MouseFx/Windows/OverlayHostWindow.*`，目录边界仍不清晰。

本阶段目标：

- 将 Overlay Host Window 具体实现物理迁移到 `Platform/windows/Overlay`；
- 在命名上明确平台归属（`Win32OverlayHostWindow`）；
- 保持 `OverlayHostService` 与业务层可见行为不变。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 用户可见效果不变；
- 变更聚焦于实现归属和依赖方向收敛。

## 3. 实施内容

### 3.1 Overlay Host Window 文件迁移与命名收敛

迁移并重命名：

- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h`
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`

到：

- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayHostWindow.h`
- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayHostWindow.cpp`

关键点：

- 类名统一为 `Win32OverlayHostWindow`；
- Win32 相关窗口生命周期、分屏 surface 渲染、TopMost 维护逻辑保持原样。

### 3.2 Win32 后端改为组合新类

修改：

- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayHostBackend.h`
- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayHostBackend.cpp`

变更：

- 成员类型从旧 `OverlayHostWindow` 切到 `Win32OverlayHostWindow`；
- `Create/Shutdown/AddLayer/RemoveLayer` 行为保持一致。

### 3.3 工程清单同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 移除旧 `MouseFx\Windows\OverlayHostWindow.*`；
- 加入 `Platform\windows\Overlay\Win32OverlayHostWindow.*`。

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 仅迁目录与命名，不改绘制算法；
- 外部入口仍通过 `IOverlayHostBackend`，回归面集中。

当前边界：

- Overlay 坐标空间逻辑仍位于 `Core/Overlay/OverlayCoordSpace.cpp`，后续需继续平台化。

## 6. 后续建议（阶段 13）

- 抽象 `OverlayCoordSpace` 服务接口；
- 将虚拟屏原点与坐标变换实现下沉到 `Platform/windows`。
