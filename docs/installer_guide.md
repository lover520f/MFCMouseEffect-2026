# 安装程序（Inno Setup）配置记录

## 需求实现
按照用户要求，安装程序实现了以下功能：

### 1. 安装前关闭正在运行的程序
- **实现方式**：在 `.iss` 脚本中配置了 `AppMutex=Global\MFCMouseEffect_SingleInstance_Mutex`。
- **效果**：Inno Setup 会在安装前检查该互斥体。如果程序正在运行，安装程序会弹出对话框提示用户关闭程序，或尝试自动关闭（`CloseApplications=yes`）。这利用了我们在代码中实现的进程单例逻辑。

### 2. 覆盖旧版本文件
- **实现方式**：利用 Inno Setup 的默认行为。
- **效果**：当安装到相同目录时，旧的文件会被自动替换或更新。在卸载时，所有由该安装程序安装的文件都会被清除。

### 3. 多语言支持
- **配置**：脚本包含了简体中文 (`ChineseSimplified`) 和英文 (`English`) 的支持。

### 4. 开机自启动选项
- **实现方式**：通过 `Registry` 节点在 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run` 写入启动项。

## 如何构建安装包
1. 确保已安装 [Inno Setup 6+](https://jrsoftware.org/isdl.php)。
2. 在 Visual Studio 中，将配置切换为 **Release | x64** 并执行 **重新生成解决方案**。
3. 打开 `Install/MFCMouseEffect.iss` 文件。
4. 在 Inno Setup 编译器中点击 `Build -> Compile` (或按 `F9`)。
5. 生成的安装包将保存在 `Install/Output/MFCMouseEffect_Setup.exe`。

## 相关文件
- [MFCMouseEffect.iss](file:///f:/language/cpp/code/MFCMouseEffect/Install/MFCMouseEffect.iss)
