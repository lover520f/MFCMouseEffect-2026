# MFC 壳解耦阶段 38：可执行目录解析复用平台服务

## 1. 背景与目标

阶段 37 已将运行时探测能力迁移到 `PlatformRuntimeEnvironment`，但 Core 内仍有重复的 EXE 路径解析：

- `MFCMouseEffect/MouseFx/Core/Config/ConfigPathResolver.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.cpp`

两处均自行调用 `GetModuleFileNameW`，导致平台细节在 Core 扩散。

本阶段目标：

- 复用 `PlatformRuntimeEnvironment` 提供的可执行目录 API；
- 删除上述两处重复 Win32 路径解析代码；
- 保持配置目录和 WASM 搜索根行为不变。

## 2. 判定

判定：`架构演进`（依赖方向优化，不改变可见行为）。

依据：

- 目录来源仍是当前进程的可执行目录；
- 失败兜底逻辑保持一致（空目录时回退到 `.` 或空路径处理）；
- `WasmPluginPaths` 的插件根计算规则不变。

## 3. 实施内容

### 3.1 ConfigPathResolver 复用平台 API

文件：

- `MFCMouseEffect/MouseFx/Core/Config/ConfigPathResolver.cpp`

变更：

- 引入 `Platform/PlatformRuntimeEnvironment.h`；
- `ResolveConfigDirectory()` 回退分支改为 `platform::GetExecutableDirectoryW()`；
- `ResolveLocalDiagDirectory()` 改为基于 `platform::GetExecutableDirectoryW()` 拼接 `.local/diag`；
- 移除本地 `GetModuleFileNameW + find_last_of` 重复逻辑。

### 3.2 WasmPluginPaths 复用平台 API

文件：

- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.cpp`

变更：

- 引入 `Platform/PlatformRuntimeEnvironment.h`；
- `ModuleDirectory()` 改为直接使用 `platform::GetExecutableDirectoryW()`；
- 删除对 `GetModuleFileNameW` 的直接调用和本地缓冲区组装代码。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与后续

已控制：

- 路径来源统一后，Core 不再持有该类 Win32 API；
- 出错兜底路径保持稳定，避免配置目录回归。

后续建议：

- 继续将 Core 内剩余 Win32 依赖（如资源加载、部分工具函数）按“接口 + 平台实现”模式迁移；
- 为 `PlatformRuntimeEnvironment` 增加跨平台实现（macOS/Linux）时复用该调用面，减少上层改动。
