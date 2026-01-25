# MFCMouseEffect

<p align="center">
  <img src="./MFCMouseEffect/res/logo_elegant.png" width="128" alt="MFCMouseEffect Logo">
</p>

**🇨🇳 中文** | **[🇬🇧 English](README.en.md)**

---

**MFCMouseEffect** 是一款轻量级、高性能的 Windows 桌面增强工具，旨在通过实时视觉反馈（波纹、粒子轨迹、文字特效等）提升您的交互体验。

### 🌟 核心特性
- **多种点击特效**：支持波纹动画、随机文字飘浮、点击爆破等多种点击反馈。
- **动态鼠标拖尾**：优雅的粒子流随鼠标移动，支持多种颜色主题（如彩虹、极光等）。
- **滚动与悬浮反馈**：不仅是点击，滚动滚轮和鼠标悬浮时也提供细腻的视觉指引。
- **极致性能**：基于 C++/MFC 开发，利用 GDI+ 进行硬件加速绘制，低 CPU 和内存消耗。
- **进程单例与托盘化**：自动确保单实例运行，支持系统托盘后台运行。

### 📸 特效展示

| | |
| :---: | :---: |
| <img src="./docs/images/setting_cn.png" width="350"><br>**设置界面** | <img src="./docs/images/ripple_concept.png" width="350"><br>**点击波纹** |
| <img src="./docs/images/trail_concept.png" width="350"><br>**粒子拖尾** | <img src="./docs/images/scroll_concept.png" width="350"><br>**滚动反馈** |
| <img src="./docs/images/hold_concept.png" width="350"><br>**长按蓄力** | <img src="./docs/images/hover_concept.png" width="350"><br>**悬停发光** |

### 🎨 主题与自定义
您可以在设置窗口中轻松切换不同的视觉主题（如彩虹、极光、霓虹等）。每种特效都可以独立开启或关闭，并根据您的个人喜好进行配置。

---

## 🛠 安装与使用

### 编译构建
1. 使用 Visual Studio 2022 打开 `MFCMouseEffect.sln`。
2. 选择 `Release | x64` 配置。
3. 执行 `生成 -> 重新生成解决方案`。
4. 运行 `x64/Release/MFCMouseEffect.exe`。

### 安装包
使用配套的 [Inno Setup 脚本](./Install/MFCMouseEffect.iss) 创建安装包。

---

## 📂 项目结构
- **MFCMouseEffect/**: MFC 核心界面与应用逻辑。
- **MouseFx/**: 特效引擎底层核心。
- **docs/**: 技术文档 ([UI 优化记录](./docs/ui_refinement.md), [单例实现记录](./docs/singleton_implementation.md))。
- **Install/**: Inno Setup 安装脚本。

---

## ⚖️ 开源协议
[MIT License](./LICENSE)

---
**⭐ 如果觉得有用，请给个 Star 支持一下！**
