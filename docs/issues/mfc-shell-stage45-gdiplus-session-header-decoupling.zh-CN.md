# MFC 壳解耦阶段 45：GdiPlusSession 头文件去平台实现暴露

## 1. 背景与目标

阶段 44 后，`MouseFx/Core/System/GdiPlusSession.h` 仍在头文件直接包含：

- `windows.h`
- `gdiplus.h`

并以内联实现方式暴露平台细节，导致 Core 头依赖链偏重。

本阶段目标：

- 将 `GdiPlusSession` 改为 `.h + .cpp` 分离；
- 头文件仅保留平台无关声明；
- 保持 AppController 的 GDI+ 生命周期行为不变。

## 2. 判定

判定：`架构演进`（封装边界优化，不改变运行语义）。

依据：

- `Startup/Shutdown` 时机与调用方不变；
- 仍使用 `GdiplusStartup/GdiplusShutdown`，仅移到实现文件；
- 双配置构建通过。

## 3. 实施内容

### 3.1 GdiPlusSession 头文件瘦身

修改：

- `MFCMouseEffect/MouseFx/Core/System/GdiPlusSession.h`

变更：

- 移除 `#include <windows.h>` 与 `#include <gdiplus.h>`；
- 将内联实现改为声明：
  - `bool Startup();`
  - `void Shutdown();`
- 内部 token 类型改为 `std::uintptr_t`，避免头文件依赖 `ULONG_PTR`。

### 3.2 新增实现文件承载平台细节

新增：

- `MFCMouseEffect/MouseFx/Core/System/GdiPlusSession.cpp`

实现要点：

- 在 `.cpp` 中包含 `gdiplus.h`；
- `Startup()` 内调用 `GdiplusStartup`；
- `Shutdown()` 内调用 `GdiplusShutdown`；
- token 在本地 `ULONG_PTR` 与类成员 `std::uintptr_t` 间做显式转换。

### 3.3 工程文件同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 新增 `MouseFx/Core/System/GdiPlusSession.cpp` 编译项。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 量化结果

`MouseFx + UI` 范围内 `#include <windows.h>`：

- `7 -> 6`

当前剩余主要集中于 GPU 渲染相关头文件。

## 6. 风险与后续

已控制：

- 调用路径仅从头内联改为链接单元，不涉及行为分支变化；
- `AppController` 持有与调用模式保持不变。

后续建议：

- 继续按模块清理 GPU 路径可下沉的头依赖；
- 评估是否将部分 GPU 运行时桥接转移到 `Platform/windows/Render` 命名域下。

