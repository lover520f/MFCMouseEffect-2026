# MFC 壳解耦阶段 37：运行时探测能力平台门面化

## 1. 背景与目标

阶段 36 后，Dawn 运行时探测与可执行目录解析仍放在 Core 路径：

- `MFCMouseEffect/MouseFx/Core/System/GpuProbeHelper.h/.cpp`

该能力直接依赖 Win32 API（`GetModuleFileNameW`、`LoadLibraryW`），属于平台实现，不应继续由 Core 层持有。

本阶段目标：

- 将 Win32 运行时探测实现迁移到 `Platform/windows`；
- 在 `Platform` 层提供统一门面，供 Core/Server 调用；
- 保持探测行为、回退路径与返回语义不变。

## 2. 判定

判定：`架构演进`（平台边界收口，不改变功能行为）。

依据：

- 探测路径优先级保持一致（`exe` 目录 -> `Runtime/Dawn` -> 仓库 `Runtime/Dawn`）；
- 返回结构 `available/reason` 语义不变；
- Web 设置接口仅改依赖方向，不改业务逻辑。

## 3. 实施内容

### 3.1 Win32 实现迁移与命名收口

从：

- `MFCMouseEffect/MouseFx/Core/System/GpuProbeHelper.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/System/Win32RuntimeEnvironment.h/.cpp`

要点：

- 命名统一到 `RuntimeEnvironment` 语义；
- 路径工具函数收口为：
  - `GetExecutableDirectoryW()`
  - `GetParentDirectoryW(...)`
- 探测入口保留为 `ProbeDawnRuntimeOnce()`。

### 3.2 新增平台门面

新增：

- `MFCMouseEffect/Platform/PlatformRuntimeEnvironment.h`
- `MFCMouseEffect/Platform/PlatformRuntimeEnvironment.cpp`

职责：

- 对上提供 `mousefx::platform::*` 统一 API；
- 在 Win32 下转发到 `mousefx::platform::windows`；
- 非 Win32 提供安全兜底返回（`runtime_probe_not_supported`）。

### 3.3 业务调用改为平台依赖

文件：

- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

变更：

- 从 `MouseFx/Core/System/GpuProbeHelper.h` 切到 `Platform/PlatformRuntimeEnvironment.h`；
- 可执行目录调用改为 `platform::GetExecutableDirectoryW()`。

### 3.4 工程文件同步

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 移除 `MouseFx/Core/System/GpuProbeHelper.*`；
- 增加 `Platform/PlatformRuntimeEnvironment.*`；
- 增加 `Platform/windows/System/Win32RuntimeEnvironment.*`；
- 统一映射到 `头文件\Platform` / `源文件\Platform`。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过（首次因 `MFCMouseEffect.exe` 占用触发 `LNK1104`，自动结束进程后重试通过）

## 5. 风险与后续

已控制：

- 保持原有探测顺序与字符串原因码，避免 Web 设置诊断行为回归；
- 非 Win32 分支具备可编译兜底，便于后续 macOS/Linux 实现补齐。

后续建议：

- 继续盘点 Core 中仍含 Win32 API 的组件，按同样模式迁移到 `Platform/windows`；
- 为 `PlatformRuntimeEnvironment` 增加最小单元测试桩（路径解析与 reason 分支）。
