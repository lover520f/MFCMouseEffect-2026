# MFC 壳解耦阶段 42：核心编码转换与 WebUI 资源加载平台化

## 1. 背景与目标

阶段 41 后，`MouseFx` 里仍有两类典型 Win32 直连能力：

- 文本编码转换（`StringUtils.cpp`、`EffectConfig*` 内的 `MultiByteToWideChar/WideCharToMultiByte`）
- WebUI 内嵌资源读取（`WebUiAssets.cpp` 内 `FindResource/LoadResource/LockResource`）

这会让 Core/Server 路径继续携带平台细节，影响后续 macOS/Linux 复用。

本阶段目标：

- 把编码转换与二进制资源读取下沉到 `Platform/windows`；
- `MouseFx` 仅依赖平台门面，不再直接调用上述 Win32 API；
- 保持既有行为不变（配置读写、WebUI 文件回退策略不变）。

## 2. 判定

判定：`架构演进`（平台边界收口，不改变产品语义）。

依据：

- 外部接口、配置字段、Web API 路由不变；
- Win32 下仍由原生 Win32 API 执行转换/资源加载；
- 非 Win32 下提供可编译兜底实现（保持接口闭合）。

## 3. 实施内容

### 3.1 新增平台编码门面与 Win32 实现

新增：

- `MFCMouseEffect/Platform/PlatformTextEncoding.h`
- `MFCMouseEffect/Platform/PlatformTextEncoding.cpp`
- `MFCMouseEffect/Platform/windows/System/Win32TextEncoding.h`
- `MFCMouseEffect/Platform/windows/System/Win32TextEncoding.cpp`

门面能力：

- `platform::Utf8ToWide`
- `platform::WideToUtf8`
- `platform::ActiveCodePageToUtf8`

说明：

- Win32 路径保持原本 `CP_UTF8/CP_ACP` 语义；
- 非 Win32 提供 `codecvt` 兜底（编译可过、行为可预期）。

### 3.2 新增平台二进制资源加载门面与 Win32 实现

新增：

- `MFCMouseEffect/Platform/PlatformBinaryResourceLoader.h`
- `MFCMouseEffect/Platform/PlatformBinaryResourceLoader.cpp`
- `MFCMouseEffect/Platform/windows/System/Win32BinaryResourceLoader.h`
- `MFCMouseEffect/Platform/windows/System/Win32BinaryResourceLoader.cpp`

门面能力：

- `platform::TryLoadEmbeddedBinaryResource`

说明：

- `WebUiAssets` 不再直接依赖 `FindResource/LoadResource/LockResource`；
- Win32 下仍使用 `RT_RCDATA` 从主模块读取资源。

### 3.3 Core/Server 调整为平台门面调用

调整文件：

- `MFCMouseEffect/MouseFx/Utils/StringUtils.cpp`
- `MFCMouseEffect/MouseFx/Utils/StringUtils.h`
- `MFCMouseEffect/MouseFx/Server/WebUiAssets.h`
- `MFCMouseEffect/MouseFx/Server/WebUiAssets.cpp`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Common.cpp`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfig.Internal.cpp`

关键点：

- `StringUtils` 不再 `#include <windows.h>`，统一走 `PlatformTextEncoding`；
- `WebUiAssets` 不再直接用 Win32 资源 API；
- `WebUiAssets` 的后缀匹配改为 ASCII 小写比较，去掉 `_stricmp` 依赖；
- `EffectConfig` 的 UTF-8/ACP 逻辑复用 `StringUtils`，减少重复转换代码。

### 3.4 工程文件同步

更新：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

内容：

- 新增上述 8 个平台文件的编译与过滤器映射。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

说明：

- 首次 Release 链接遇到 `LNK1104`（目标 exe 被占用），已按协作约定自动结束占用进程并重试通过。

## 5. 量化结果

`MouseFx + UI` 范围：

- `#include <windows.h>`：`10 -> 8`（减少 2 处）
- `MultiByteToWideChar/WideCharToMultiByte/CP_*`：从 `MouseFx` 主路径清理，当前仅 UI 设置窗口仍保留 6 处（后续可继续平台化）。

## 6. 风险与后续

已控制：

- 编码转换语义在 Win32 保持一致；
- WebUI 资源读取逻辑仅替换实现层，不改路由与文件优先级策略。

后续建议：

- 继续处理 `UI/Settings/SettingsWnd.cpp` 的编码 API（可转到 `PlatformTextEncoding`）；
- 继续收口 `MonitorUtils.h` 与 GPU 相关头文件的 Win32 依赖。

