# 安装包（Inno Setup）更新笔记（2026-02-04）

## 变更摘要
- 安装包名称包含版本号与架构：`MFCMouseEffect_{version}_Setup_x64.exe`。
- 发行者/链接指向 GitHub 仓库（Publisher: `sqmw`，URL: `https://github.com/sqmw/MFCMouseEffect`）。
- 默认复制浏览器设置页资源：`webui/`（与 exe 同目录的本地 HTTP 服务器静态文件）。
- 追加随包文档：`LICENSE`、`README.md`、`README.en.md`（若存在则拷贝）。
- 支持中英文安装界面（ChineseSimplified / English）；保留上次的安装目录和语言。
- 安装/卸载前自动结束运行中的 `MFCMouseEffect.exe`，避免文件占用。
- 如果本机未安装 `ChineseSimplified.isl` 语言包，编译器会自动仅使用英文，避免 `Couldn't open include file` 报错。

## 使用提示
- 生成前需先构建 `Release|x64`，确保 `x64\\Release\\webui` 已由 PostBuild 拷贝。
- 安装包图标取自 `MFCMouseEffect\\res\\MFCMouseEffect.ico`（Install 目录相对路径）。
- 如需便携版，可直接分发 `x64\\Release\\` 目录；此 iss 方案适用于标准安装版。
