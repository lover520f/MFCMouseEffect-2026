# MFC 壳解耦阶段 43：SettingsWnd 编码 API 统一到 Core 工具层

## 1. 背景与目标

阶段 42 后，`MouseFx` 内的编码转换已平台化，但 UI 仍存在直接 Win32 编码 API：

- `UI/Settings/SettingsWnd.cpp` 内多处 `MultiByteToWideChar/WideCharToMultiByte`。

这会让 UI 继续耦合平台细节，并与 Core 的编码策略产生重复实现。

本阶段目标：

- 将 `SettingsWnd` 的 UTF-8/UTF-16 转换统一到 `MouseFx/Utils/StringUtils`；
- 删除该文件内重复的 Win32 编码转换逻辑；
- 保持设置窗口行为不变（文本框读写与持久化不变）。

## 2. 判定

判定：`架构演进`（代码去重 + 接口统一，不改交互语义）。

依据：

- 仍由同一套 `StringUtils`（阶段 42 已接入平台门面）执行转换；
- UI 控件字段、保存时机、后端数据结构均未变更；
- Debug/Release 构建通过。

## 3. 实施内容

修改文件：

- `MFCMouseEffect/UI/Settings/SettingsWnd.cpp`

关键改动：

- 新增 `Utf8ToCString`、`CStringToUtf8` 本地辅助函数；
- `OnCreate` 初始化文本内容时，改为 `Utf8ToCString(model_.textContent)`；
- `CaptureUI` 读取文本框时，改为 `CStringToUtf8(wText)`；
- `SyncFromBackend` 同步文本时，改为 `Utf8ToCString(model_.textContent)`；
- 移除原文件内所有 `MultiByteToWideChar/WideCharToMultiByte/CP_UTF8` 调用。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 量化结果

`MouseFx + UI` 范围：

- `MultiByteToWideChar/WideCharToMultiByte/CP_*`：`6 -> 0`
- `#include <windows.h>`：维持 `8`（当前集中在 GPU/Monitor/GDI+ 相关路径）。

## 6. 风险与后续

已控制：

- UI 文本转换能力复用 Core 工具层，避免未来多处行为分叉；
- 改动集中在转换逻辑，不影响窗口布局与消息流。

后续建议：

- 继续将 `MonitorUtils` 与 GPU 相关头依赖按模块下沉至 `Platform/windows`；
- 在后续阶段逐步把 `UI` 与 `Platform` 的边界做更细粒度收口。

