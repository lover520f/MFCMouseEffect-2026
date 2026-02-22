# MFC 壳解耦阶段 40：自动化应用目录与原生目录选择器平台化

## 1. 背景与目标

在阶段 39 后，`MouseFx/Core/System` 仍保留两块明显 Win32/COM 绑定能力：

- `ApplicationCatalogScanner`（KnownFolder + ShellLink 扫描）
- `NativeFolderPicker`（`IFileOpenDialog` 目录选择）

这两块能力被 `WebSettingsServer` 直接使用，导致 Core 路径继续承载平台实现细节。

本阶段目标：

- 将上述能力迁移到 `Platform/windows/System`；
- 在 `Platform` 层补统一门面；
- 保持 Web API 行为不变（自动化 app catalog 与 wasm 文件夹选择）。

## 2. 判定

判定：`架构演进`（平台边界收口，不改变功能语义）。

依据：

- API 入参/出参与 JSON 字段未改；
- Win32 下仍使用原有 COM/Shell 实现；
- 非 Win32 下门面返回显式不支持，避免上层耦合 Win32 头文件。

## 3. 实施内容

### 3.1 Win32 实现迁移

从：

- `MFCMouseEffect/MouseFx/Core/System/ApplicationCatalogScanner.h/.cpp`
- `MFCMouseEffect/MouseFx/Core/System/NativeFolderPicker.h/.cpp`

迁移到：

- `MFCMouseEffect/Platform/windows/System/Win32ApplicationCatalogScanner.h/.cpp`
- `MFCMouseEffect/Platform/windows/System/Win32NativeFolderPicker.h/.cpp`

变更：

- 命名空间改为 `mousefx::platform::windows`；
- 类型与方法命名改为 Win32 明确语义（`Win32*`）。

### 3.2 新增平台门面

新增：

- `MFCMouseEffect/Platform/PlatformApplicationCatalog.h/.cpp`
- `MFCMouseEffect/Platform/PlatformNativeFolderPicker.h/.cpp`

门面 API：

- `platform::ScanApplicationCatalog()`
- `platform::PickFolder(...)`

行为：

- Win32：转发到 `Platform/windows/System` 实现；
- 非 Win32：返回空目录或 `native_folder_picker_not_supported`。

### 3.3 WebSettingsServer 改为平台调用

文件：

- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`

变更：

- 删除对 `MouseFx/Core/System/ApplicationCatalogScanner.h`、`NativeFolderPicker.h` 依赖；
- 改为 `Platform/PlatformApplicationCatalog.h`、`Platform/PlatformNativeFolderPicker.h`；
- 自动化 app catalog 和 wasm 目录选择接口改为调用平台门面。

### 3.4 工程文件同步

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 移除旧 `MouseFx/Core/System/*` 两组文件；
- 新增 `Platform/*` 门面文件与 `Platform/windows/System/*` 实现文件；
- filters 统一映射到 `头文件\Platform` / `源文件\Platform`。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与后续

已控制：

- 业务层路由逻辑不改，仅替换依赖方向；
- Win32 现有实现逻辑原样迁移，风险主要是命名空间与工程文件映射，已通过构建验证覆盖。

后续建议：

- 继续梳理 `MouseFx` 目录内剩余 Win32 API（如 `WebUiAssets`、编码转换、渲染头文件）；
- 按“平台门面 + Win32实现 + 非Win32兜底”模式持续收口。
