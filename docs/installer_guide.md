# 安装程序（Inno Setup）配置记录

## 需求实现
按照用户要求，安装程序实现了以下功能：

### 1. 安装前关闭正在运行的程序
- **实现方式**：在 `.iss` 脚本中配置了 `AppMutex=Global\MFCMouseEffect_SingleInstance_Mutex`。
- **效果**：Inno Setup 会在安装前检查该互斥体。如果程序正在运行，安装程序会弹出对话框提示用户关闭程序，或尝试自动关闭（`CloseApplications=yes`）。这利用了我们在代码中实现的进程单例逻辑。

### 2. 覆盖旧版本文件与配置持久化
- **实现方式**：
    - **文件覆盖**：利用 Inno Setup 的默认行为处理程序文件。
    - **配置分离**：修改了程序逻辑，现在配置文将优先保存在专用的用户数据目录 `%AppData%\MFCMouseEffect\config.json` 中。
    - **运行时 DLL**：安装包包含 `webgpu_dawn.dll` 与 `d3dcompiler_47.dll`，避免运行时缺失。
- **效果**：即使安装在 `Program Files` 等受限目录下，设置也能正常保存和读取。安装包中的 `config.json` 仅作为默认模板，且配置了 `onlyifdoesntexist` 标志，不会覆盖用户已有的个性化设置。

### 3. 多语言支持
- **配置**：脚本包含了英文 (`English`) 的首选支持，并预留了中文接口。

### 4. 默认创建快捷方式与开机自启动
- **实现方式**：在脚本任务配置中移除了 `unchecked` 标志。
- **效果**：安装程序在运行时会默认勾选“创建桌面快捷方式”和“开机自启动”选项。

## 如何构建安装包
1. 确保已安装 [Inno Setup 6+](https://jrsoftware.org/isdl.php)。
2. 在 Visual Studio 中，将配置切换为 **Release | x64** 并执行 **重新生成解决方案**。
3. 打开 `Install/MFCMouseEffect.iss` 文件。
4. 在 Inno Setup 编译器中点击 `Build -> Compile` (或按 `F9`)。
5. 生成的安装包将保存在 `Install/Output/MFCMouseEffect_Setup.exe`。

## 相关文件
- [MFCMouseEffect.iss](file:///f:/language/cpp/code/MFCMouseEffect/Install/MFCMouseEffect.iss)

### 5. 文件版本显示问题修复
- **问题**：打包出来的安装程序文件版本默认显示为 `0.0.0.0`。
- **原因**：Inno Setup 默认不填充 exe 的版本信息资源，除非显式指定 `VersionInfoVersion` 等指令。
- **解决方案**：在 `[Setup]` 段中添加了以下指令：
  ```ini
  VersionInfoVersion={#MyAppVersion}
  VersionInfoCompany={#MyAppPublisher}
  VersionInfoDescription={#MyAppName} Setup
  VersionInfoProductVersion={#MyAppVersion}
  VersionInfoCopyright=Copyright (C) 2026 {#MyAppPublisher}
  VersionInfoProductName={#MyAppName}
  ```
  这确保了在 Windows 资源管理器中右键查看属性时，能正确显示版本号（如 `1.3.0`）及公司信息。

### 6. 安装权限与注册表警告修复
- **问题**：编译时警告 `The [Setup] section directive "PrivilegesRequired" is set to "admin" but per-user areas ...`。
- **原因**：安装程序强制要求管理员权限，但脚本试图写入 `HKCU`（当前用户注册表），这被称为“per-user area”。虽然 Inno Setup 允许这样做，但它警告这可能不会对所有用户生效（仅对运行安装程序的管理员生效）。
- **解决方案**：将注册表根键从 `HKCU` 改为 `HKA`（Auto）。
  ```ini
  Root: HKA; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ...
  ```
  `HKA` 会根据安装模式自动选择：
  - 如果是以管理员权限安装（All Users），它会写入 `HKLM`（全系统启动）。
  - 如果是非管理员安装（Current User），它会写入 `HKCU`。
  由于我们设置了 `PrivilegesRequired=admin`，这通常意味着写入 `HKLM`，从而消除了警告并确保逻辑正确。
  Due to setting `PrivilegesRequired=admin`, this usually means writing to `HKLM`, eliminating the warning and ensuring correct logic.

### 7. 架构标识符警告修复
- **问题**：编译时警告 `Architecture Identifiers topic ... most cases`。
- **原因**：Inno Setup 6.3+ 将 `x64` 标识符标记为已弃用（deprecated），建议使用 `x64compatible` 以更好地支持现代 64 位架构（包括 ARM64）。
- **解决方案**：更改标识符配置。
  ```ini
  ArchitecturesAllowed=x64compatible
  ArchitecturesInstallIn64BitMode=x64compatible
  ```
