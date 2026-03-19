# MFCMouseEffect

<p align="center">
  <img src="./MFCMouseEffect/res/logo_elegant.png" width="128" alt="MFCMouseEffect Logo">
</p>

<p align="center">
  <a href="https://github.com/sqmw/MFCMouseEffect/releases/latest"><img src="https://img.shields.io/badge/release-latest-blue" alt="release"></a>
  <img src="https://img.shields.io/badge/status-active%20development-green" alt="status">
  <img src="https://img.shields.io/badge/license-MIT-brightgreen" alt="license">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20core%20lane%20%7C%20Linux%20gate-lightgrey" alt="platform">
</p>

**🇨🇳 中文** | **[🇬🇧 English](README.en.md)**

---

`MFCMouseEffect` 是一个桌面输入可视化与交互反馈引擎：
- 鼠标特效（点击/拖尾/滚轮/长按/悬停）
- 键鼠输入指示器（鼠标 + 键盘）
- 自动化映射（鼠标动作 + 鼠标手势 -> 快捷键注入）
- WASM 特效插件运行时（加载、重载、诊断、导入导出）
- 统一 Web 设置界面（Svelte，跨平台共享）

## 效果预览

| | |
| :---: | :---: |
| <img src="./docs/images/setting_cn.png" width="340"><br>设置页示意 | <img src="./docs/images/ripple_concept.png" width="340"><br>点击波纹 |
| <img src="./docs/images/trail_concept.png" width="340"><br>拖尾效果 | <img src="./docs/images/scroll_concept.png" width="340"><br>滚轮反馈 |
| <img src="./docs/images/hold_concept.png" width="340"><br>长按反馈 | <img src="./docs/images/hover_concept.png" width="340"><br>悬停反馈 |

## 当前平台状态

| 平台 | 状态 | 说明 |
| :--- | :--- | :--- |
| Windows 10+ | 稳定主线 | 完整能力集，继续保持回归兼容 |
| macOS | 主开发线（core lane） | 特效 + 输入指示 + 自动化/手势 + WASM 合同持续增强 |
| Linux | 跟随线 | 以编译门禁 + 合同回归为主，不作为当前完整体验主线 |

> 备注：当前迭代优先级是 `macOS mainline first`，同时要求不破坏 Windows 行为。

## 能力总览

### 1) 特效系统（Effects）
- 五类交互通道：`click / trail / scroll / hold / hover`
- 类型归一化与配置映射在 Win/mac 之间对齐，降低语义漂移
- macOS 侧覆盖持续增强（含 `trail line` 连续性、`click=text` 回退语义等）
- WebSettings 提供类型切换、参数调节、诊断快照

### 2) 键鼠输入指示（Input Indicator）
- 支持鼠标点击、滚轮、键盘标签显示
- 支持相对/绝对定位、多屏目标屏选择和自定义偏移
- 提供平台能力探针与回归接口，保证可观测性

### 3) 自动化与手势识别（Automation + Gesture）
- 鼠标动作映射：左/右/中键、滚轮上下 -> 快捷键
- 手势映射：拖拽方向链（如 `up_right`、`down_left_up`）-> 快捷键
- 可配置手势触发键、最小轨迹距离、采样步长、最大方向段数
- 支持应用作用域（all/selected）与优先级匹配策略

### 4) WASM 特效插件（WASM Runtime）
- 插件清单加载、重载、目录导入、批量导出
- 支持 `error_code` 级别错误模型与 UI 映射
- 提供运行时诊断（budget、parse、render、load-failure stage/code）
- 具备 test-gated API 与回归脚本，便于非交互验证

### 5) Web 设置界面（Shared WebSettings）
当前设置页按能力拆分为独立模块：
- `General`
- `Active Effects`
- `Input Indicator`
- `Text Content (Click/Text)`
- `Automation Mapping`
- `Effect Plugins (WASM)`
- `Trail Tuning`

## 快速开始

### Windows（Visual Studio）
1. 用 Visual Studio 2026 打开 `MFCMouseEffect.slnx`
2. 选择 `Release | x64` 并重建
3. 运行 `x64/Release/MFCMouseEffect.exe`

### macOS（日常开发快捷入口）
```bash
# 编译 + 启动 core host（默认 30 分钟自动退出）
./mfx run

# 直接启动（跳过 core/WebUI 编译）
./mfx run-no-build

# 30 秒自动退出（便于快速手测）
./mfx run-no-build --seconds 30

# 特效类型等价自检
./mfx effects

# 特效回归套件（推荐日常）
./mfx verify-effects

# 全量 POSIX 回归套件
./mfx verify-full

# 兼容旧命令（仍可用）
./mfx start
./mfx fast
```

## 回归与自检入口

```bash
# 全量 POSIX 套件（scaffold + core + linux gate）
./tools/platform/regression/run-posix-regression-suite.sh --platform auto

# 特效聚焦套件
./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto

# WASM 聚焦套件
./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto
```

```bash
# macOS WebSettings 手测入口
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60

# macOS WASM 运行时自检
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build

# macOS 自动化注入自检
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
```

## 目录概览

- `MFCMouseEffect/MouseFx`：核心引擎（特效、输入、自动化、WASM、WebSettings Server）
- `MFCMouseEffect/Platform`：平台实现（Windows/macOS/Linux）
- `MFCMouseEffect/WebUIWorkspace`：Svelte 设置页源码
- `tools/platform/regression`：跨平台回归脚本
- `tools/platform/manual`：macOS 手测/自检脚本
- `docs`：架构、重构、问题与回归文档

## 文档入口

- 文档索引（中文）：`./docs/README.zh-CN.md`
- Agent 当前上下文：`./docs/agent-context/current.md`
- 路线图状态快照：`./docs/refactoring/phase-roadmap-macos-m1-status.md`

## 许可证

[MIT License](./LICENSE)

---
<p align="center"><b>觉得有用请点个 <a href="https://github.com/sqmw/MFCMouseEffect">Star ⭐</a>，也欢迎在 Issues/Discussions 留言反馈。</b></p>
