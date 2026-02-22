# Stage52 - Shared D2D 工厂平台化迁移

## 判定

`架构债务`：`MouseFx/Utils/D2DFactory.*` 是 Windows D2D/DWrite 专用实现，但放在 `MouseFx` 通用目录，且当前仅 `Platform/windows/Effects/TextWindow.cpp` 使用，存在平台边界漂移。

## 目标

1. 将共享 D2D/DWrite 工厂实现下沉到 `Platform/windows`。
2. 清理 `MouseFx` 目录中最后的 D2D 头依赖。
3. 不改变文本渲染行为，仅收敛目录边界。

## 变更摘要

### 1) 文件迁移与命名收敛

从：

- `MFCMouseEffect/MouseFx/Utils/D2DFactory.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/Graphics/Win32D2DFactory.h/.cpp`

说明：

- 保留 `SharedD2D1Factory()` / `SharedDWriteFactory()` 接口语义，避免调用侧行为变化。

### 2) 引用路径同步

更新：

- `MFCMouseEffect/Platform/windows/Effects/TextWindow.cpp`
- `MFCMouseEffect/Platform/windows/Graphics/Win32D2DFactory.cpp`

### 3) 工程与过滤器同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 头文件与编译项切换到 `Platform/windows/Graphics/Win32D2DFactory.*`。
- 添加对应 filters 条目，IDE 目录结构与平台边界一致。

## 验证

1. `Release|x64` 构建通过：`0 error / 0 warning`。
2. 构建日志 `tlog` 清理显示旧 `MouseFx/Utils/D2DFactory.cpp` 已从工程移除。
3. 扫描确认 `MouseFx` 目录无 `#include <windows.h>/<d3d11.h>/<d2d1.h>/<dcomp.h>/<dxgi*>` 直接依赖。

## 收益

1. `MouseFx` 通用目录与 Windows 图形实现完成进一步解耦。
2. 文本渲染共享工厂归位到平台层，后续新增 macOS/Linux 图形工厂时边界更清晰。
