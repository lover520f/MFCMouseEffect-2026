# MFCMouseEffect

<p align="center">
  <img src="./MFCMouseEffect/res/logo_elegant.png" width="128" alt="MFCMouseEffect Logo">
</p>

<p align="center">
  <a href="../../releases/latest"><img src="https://img.shields.io/badge/release-latest-blue" alt="release"></a>
  <img src="https://img.shields.io/badge/status-beta-green" alt="status">
  <img src="https://img.shields.io/badge/license-MIT-brightgreen" alt="license">
  <img src="https://img.shields.io/badge/platform-Windows%2010%2B-lightgrey" alt="platform">
</p>

**🇨🇳 中文** | **[🇬🇧 English](README.en.md)**

---

一款轻量、高性能的 Windows 桌面特效工具，为点击、拖尾、滚轮、长按、悬停提供实时视觉反馈（波纹、粒子、文字等）。

## ✨ 特色
- 全局低级鼠标钩子 + GDI+ 分层窗口，跟手流畅，资源占用低。
- 主题可选（霓虹 / 极简 / 游戏感等），各特效独立切换与持久化。
- 托盘模式常驻、后台模式（由父进程 stdin 控制），配置文件与 exe 同目录。
- 设置页（浏览器）：托盘菜单打开，内置本地 loopback server，支持中英切换与高级参数调节（配置写回 `config.json`）。

## 📸 效果预览
| | |
| :---: | :---: |
| <img src="./docs/images/setting_cn.png" width="340"><br>设置窗口（Legacy） | <img src="./docs/images/ripple_concept.png" width="340"><br>点击波纹 |
| <img src="./docs/images/trail_concept.png" width="340"><br>粒子拖尾 | <img src="./docs/images/scroll_concept.png" width="340"><br>滚轮指引 |
| <img src="./docs/images/hold_concept.png" width="340"><br>长按蓄力 | <img src="./docs/images/hover_concept.png" width="340"><br>悬停发光 |

## 🆕 最近修复/改进
- 设置窗口默认居中、拖动不闪烁；内部控件改回原生样式，去除冗余底色。
- 虚拟/平板副屏坐标偏移：已启用坐标归一化兜底（2026-01），大多数虚拟副屏场景已对齐。详见 `docs/issues/virtual-display-coordinates.zh-CN.md`。

## ⬇️ 下载
- 直接下载（最新版本）：[Releases](../../releases/latest)
- 历史版本：[All releases](../../releases)

## 📦 安装与运行
1. 使用 Visual Studio 2026 打开 `MFCMouseEffect.slnx`。
2. 选择 `Release | x64`，执行“重新生成解决方案”。
3. 运行 `x64/Release/MFCMouseEffect.exe`。托盘模式下右键可退出，非 background 模式右键菜单“设置...”会打开浏览器设置页。

## 🖥️ 使用说明
- 语言/主题/各特效切换：通过浏览器设置页；配置写入同目录 `config.json`。
- 管理员窗口捕获：如需对管理员窗口生效，请以管理员身份运行本程序。
- 背景模式：无托盘/界面，完全由父进程通过 stdin JSON 控制。

## 📑 文档与演示
- 详细文档：`./docs/README.zh-CN.md`
- 虚拟副屏坐标问题：`./docs/issues/virtual-display-coordinates.zh-CN.md`
- 示例截图：`./docs/images/setting_cn.png` 等

## 🧭 社区与推广（仓库维护建议）
- 在仓库 About 中填写 Description 与 Topics（如：mouse-effect, ripple, tray, mfc, windows, overlay）。
- 设置 Social Preview，并在 README 顶部保留徽章与示例图。
- 开启 Discussions，准备 3~5 个 “good first issue”：
  - 调整阈值/日志：为坐标归一化增加可配置阈值与调试输出。
  - 新主题配色：添加一套科幻霓虹主题。
  - 轻量化安装包：精简 Inno Setup 选项并新增便携版 ZIP。
  - 性能对比：提供不同 DPI/帧率下的 CPU 占用基准。
  - 文档补充：英文版 Troubleshooting 与 FAQ。

## 💖 赞助与支持
如果您觉得这个项目对您有所帮助，欢迎通过以下方式支持作者的后续开发。您的支持是我持续改进的动力！

| 支付宝 (Alipay) | 微信支付 (WeChat Pay) |
| :---: | :---: |
| <img src="./docs/images/alipay_qr.png" width="240"><br>扫码加鸡腿 | <img src="./docs/images/wechat_qr.png" width="240"><br>请作者喝咖啡 |

## ⚖️ 许可
[MIT License](./LICENSE)

---
<p align="center"><b>觉得有用请点个 <a href="https://github.com/sqmw/MFCMouseEffect">Star ⭐</a>，也欢迎在 Issues/Discussions 留言反馈。</b></p>
