# MFC 壳解耦阶段 46：GPU 头文件显式 windows.h 归零

## 1. 背景与目标

阶段 45 后，`MouseFx` 范围内剩余的 `windows.h` 几乎都集中在 GPU 渲染相关头文件。

本阶段目标：

- 不改变 GPU 路径行为前提下，去掉这些头里的显式 `#include <windows.h>`；
- 对必须跨头保存的 Win32 句柄，改为平台无关存储类型；
- 保持 Debug/Release 构建通过。

## 2. 判定

判定：`架构演进`（头依赖降噪，不改运行策略）。

依据：

- 渲染/计算逻辑未改，仅做类型存储与 API 调用处显式转换；
- 所有改动经双配置构建验证；
- 运行时关键路径（D3D11 创建、事件/线程等待、D2D 渲染）保持原语义。

## 3. 实施内容

### 3.1 去掉显式 windows.h 的头文件

修改：

- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldGpuV2ComputeEngine.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2ComputeEngine.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2D2DBackend.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2Presenter.h`
- `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.h`

处理方式：

- 删除 `#include <windows.h>`；
- 保留 D3D/D2D/COM 等必要头；
- 在需要处用平台无关成员承载句柄（见下）。

### 3.2 句柄字段平台无关化

修改：

- `FluxFieldGpuV2ComputeEngine.h`：`HMODULE d3d11Module_` -> `void* d3d11Module_`
- `QuantumHaloGpuV2ComputeEngine.h`：`HMODULE d3d11Module_` -> `void* d3d11Module_`
- `HoldQuantumHaloGpuV2DirectRuntime.h`：`HANDLE stopEvent_` -> `void* stopEvent_`

对应实现文件：

- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldGpuV2ComputeEngine.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2ComputeEngine.cpp`
- `MFCMouseEffect/MouseFx/Effects/HoldQuantumHaloGpuV2DirectRuntime.cpp`

在调用 Win32 API 时显式转换回原生类型（如 `static_cast<HMODULE>(...)`、`static_cast<HANDLE>(...)`）。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 量化结果

`MouseFx + UI` 范围显式 `#include <windows.h>`：

- `6 -> 0`

说明：

- 这里是“显式包含”归零；部分 Win32 类型仍通过 DirectX/系统头的传递包含可用，这是预期。

## 6. 风险与后续

已控制：

- API 调用处均保持显式转换，避免行为偏差；
- 句柄生命周期逻辑（LoadLibrary/FreeLibrary、CreateEvent/CloseHandle）未改。

后续建议：

- 继续评估 GPU 模块是否可按 `Platform/windows/Render` 目录进一步收口；
- 做一轮 clang-tidy/静态检查，确认句柄转换处无潜在空值/生命周期风险。

