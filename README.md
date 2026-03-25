# MFCMouseEffect

<p align="center">
  <img src="./MFCMouseEffect/res/logo_elegant.png" width="128" alt="MFCMouseEffect Logo">
</p>

<p align="center">
  <a href="https://github.com/sqmw/MFCMouseEffect/stargazers"><img src="https://img.shields.io/github/stars/sqmw/MFCMouseEffect?style=for-the-badge" alt="stars"></a>
  <a href="https://github.com/sqmw/MFCMouseEffect/releases/latest"><img src="https://img.shields.io/github/v/release/sqmw/MFCMouseEffect?style=for-the-badge" alt="release"></a>
  <a href="https://github.com/sqmw/MFCMouseEffect/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-brightgreen?style=for-the-badge" alt="license"></a>
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey?style=for-the-badge" alt="platform">
</p>

<p align="center">
  一个面向桌面输入反馈、输入可视化、自动化映射与 WASM 扩展的跨平台引擎。
</p>

<p align="center">
  <a href="https://github.com/sqmw/MFCMouseEffect">Star</a> ·
  <a href="https://github.com/sqmw/MFCMouseEffect/issues">Issues</a> ·
  <a href="https://github.com/sqmw/MFCMouseEffect/releases">Releases</a> ·
  <a href="./docs/README.zh-CN.md">Docs</a>
</p>

**🇨🇳 中文** | **[🇬🇧 English](README.en.md)**

---

## 项目概览

`MFCMouseEffect` 不只是“鼠标点击特效”。

它正在演进成一个完整的桌面交互反馈平台，核心覆盖：
- 鼠标特效：`click / trail / scroll / hold / hover`
- 光标装饰：`cursor decoration` 原生与 WASM 双路线
- 输入指示器：鼠标点击、滚轮、键盘组合键可视化
- 自动化映射：鼠标动作、滚轮、手势映射到快捷键注入
- WASM 插件运行时：支持 effect / indicator 双 surface
- 统一 Web 设置界面：跨平台共享、可观测、可调试
- Mouse Companion：插件化路线下的桌面宠物/跟随 companion 能力

如果你想做这些方向，这个项目会很对味：
- 给录屏、演示、教程增加更专业的输入反馈
- 做可扩展的桌面交互效果，而不是写死几个动画
- 在 C++ 宿主里引入受控、可回退、可诊断的 WASM 能力
- 在 Windows 与 macOS 之间做可持续演进的跨平台桌面架构

## 核心特点

- 宿主负责渲染与资源边界，插件只做逻辑计算，稳定性更高
- Win/mac 共用核心语义与设置面，减少平台行为漂移
- WebSettings、诊断、回归脚本、自检入口是一起设计的，不是后补
- 支持渐进式扩展：内建效果可用，WASM 插件可叠加，原生回退可兜底
- 架构上保持模块分层：Core、Platform、Server、WebUI、Tools、Docs 各自职责清晰

## 主图预览

| 功能 | 主图 | 功能 | 主图 |
| :--- | :--- | :--- | :--- |
| Cursor Effects | <img src="./docs/images/ripple_concept.png" width="360" alt="Cursor Effects 主图"> | Cursor Decoration | <img src="./docs/images/placeholder_cursor_decoration.png" width="360" alt="Cursor Decoration 主图占位"> |
| Input Indicator | <img src="./docs/images/placeholder_input_indicator.png" width="360" alt="Input Indicator 主图占位"> | Automation Mapping | <img src="./docs/images/placeholder_automation_mapping.png" width="360" alt="Automation Mapping 主图占位"> |
| WASM Plugin Runtime | <img src="./docs/images/placeholder_wasm_plugin_runtime.png" width="360" alt="WASM Plugin Runtime 主图占位"> | Plugin Management | <img src="./docs/images/placeholder_plugin_management.png" width="360" alt="Plugin Management 主图占位"> |
| Mouse Companion | <img src="./docs/images/placeholder_mouse_companion.webp" width="360" alt="Mouse Companion 主图占位"> | Shared WebSettings | <img src="./docs/images/setting_cn.png" width="360" alt="WebSettings 主图"> |

<details>
<summary>功能全景与细节截图（展开查看）</summary>

## 功能全景

### 1. Cursor Effects

支持五类核心交互通道：
- `click`：点击波纹、点击文本、点击图片等
- `trail`：拖尾、粒子、线段连续轨迹
- `scroll`：滚轮方向与节奏反馈
- `hold`：长按蓄力、环形进度、持续态反馈
- `hover`：悬停视觉响应

亮点：
- 五条通道是独立能力面，不是单一路径的样式切换
- Win/mac 在类型归一化、配置映射和语义层面持续对齐
- 可通过 WebSettings 调整类型、参数和行为
- 部分效果支持 WASM 插件承接，而不是只能走内建实现

### 2. Cursor Decoration

除了点击和拖尾，这个项目还支持独立的光标装饰层：
- 原生 `ring / orb` 装饰
- 独立于五条主特效通道存在
- 支持走内建路线，也支持切到 `cursor_decoration` WASM lane
- 会与应用黑名单、原生回退、插件启停保持一致的可控行为

### 3. Input Indicator

输入指示器支持：
- 鼠标左键 / 右键 / 中键状态
- 滚轮方向与连击语义，例如 `W+ x2`
- 键盘事件与组合键标签，例如 `Cmd+Tab`
- 相对 / 绝对定位、多屏目标定位、偏移配置
- 原生 fallback 与 WASM indicator surface 双路线

### 4. Automation Mapping

自动化映射不是简单热键表，而是完整输入触发层：
- 鼠标动作映射：左/右/中键、滚轮上/下
- 鼠标手势映射：方向链，如 `up_right`、`down_left_up`
- 可配置手势触发键、采样步长、最小轨迹距离、最大方向段数
- 支持应用作用域匹配和确定性优先级规则
- 提供手势绘制、保存、自检与回归脚本

### 5. WASM Plugin Runtime

WASM 路线是这个项目最有辨识度的部分之一：
- 支持 `effects` 与 `indicator` 两类 surface
- 支持 manifest 加载、重载、导入、导出
- 支持预算控制、命令校验、错误码与阶段诊断
- 支持 transient 与 retained 两类渲染语义
- 宿主拥有渲染执行权，插件不直接控制窗口和资源

当前命令面已经覆盖很多高价值原语：
- `spawn_text`
- `spawn_image`
- `spawn_pulse`
- `spawn_polyline`
- `spawn_path_stroke`
- `spawn_path_fill`
- `spawn_ribbon_strip`
- `spawn_glow_batch`
- `spawn_sprite_batch`
- `spawn_quad_batch`
- retained emitter / trail / quad field / group primitives

### 6. Plugin Management

项目内已经形成独立插件管理能力：
- 统一 `Plugin Management` 入口
- 支持 effect lane、indicator lane、cursor decoration lane 的策略绑定
- 支持插件 manifest 路径、启停、错误状态和回退状态观察
- 设置页能反映真实后端状态，而不是只改前端表单

### 插件使用指引

插件是核心能力之一，建议直接从以下入口上手：
- 使用模板：`examples/wasm-plugin-template/README.md`
- 主文档：`docs/architecture/custom-effects-wasm-route.zh-CN.md`
- ABI 说明：`docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md`
- 加载入口：设置页 `Plugin Management` -> 选择 manifest -> Apply

<details>
<summary>更多使用提示（展开查看）</summary>

- 根据插件目标选择 surface：`effects` 或 `indicator`
- 如果 Apply 后回滚，优先检查 manifest 路径与加载状态
- 设置页会显示插件诊断与回退状态，先看这里再排查

</details>

### 7. Mouse Companion

`Mouse Companion` 是当前非常有潜力的一条扩展路线：
- 目标是插件优先、跨平台可演进的桌面 companion 能力
- macOS 已有 Phase1 visual host
- Windows 当前是 Phase1.5，强调插件宿主、renderer seam 与可诊断 runtime
- 当前主线不是“随便做个宠物动画”，而是建立长期可扩展的 companion 架构

### 8. Shared WebSettings

当前设置页按能力拆分，结构更清晰：
- `General`
- `Mouse Companion`
- `Cursor Effects`
- `Input Indicator`
- `Automation Mapping`
- `Plugin Management`

优点：
- 配置路径统一
- 页面组织与后端状态同步
- 更适合做平台共享、能力扩展和调试诊断

## 预览与截图占位

下面这些位置你后面直接替换成图片即可，结构我已经先给你搭好了。

### 总览预览

| 模块 | 当前展示 | 模块 | 当前展示 |
| :--- | :--- | :--- | :--- |
| 设置页 | <img src="./docs/images/setting_cn.png" width="360" alt="设置页示意"> | 点击特效 | <img src="./docs/images/ripple_concept.png" width="360" alt="点击特效示意"> |
| 拖尾特效 | <img src="./docs/images/trail_concept.png" width="360" alt="拖尾特效示意"> | 滚轮特效 | <img src="./docs/images/scroll_concept.png" width="360" alt="滚轮特效示意"> |
| 长按特效 | <img src="./docs/images/hold_concept.png" width="360" alt="长按特效示意"> | 悬停特效 | <img src="./docs/images/hover_concept.png" width="360" alt="悬停特效示意"> |

<details>
<summary>更多细节图（占位，可继续追加）</summary>

| 场景 | 细节图 | 场景 | 细节图 |
| :--- | :--- | :--- | :--- |
| 输入指示器 | <img src="./docs/images/placeholder_input_indicator.png" width="360" alt="输入指示器细节占位"> | 自动化映射 | <img src="./docs/images/placeholder_automation_mapping.png" width="360" alt="自动化映射细节占位"> |
| WASM 运行时 | <img src="./docs/images/placeholder_wasm_plugin_runtime.png" width="360" alt="WASM 运行时细节占位"> | 插件管理 | <img src="./docs/images/placeholder_plugin_management.png" width="360" alt="插件管理细节占位"> |
| 光标装饰 | <img src="./docs/images/placeholder_cursor_decoration.png" width="360" alt="光标装饰细节占位"> | Mouse Companion | <img src="./docs/images/placeholder_mouse_companion.png" width="360" alt="Mouse Companion 细节占位"> |

</details>

</details>

## 平台状态

| 平台 | 状态 | 说明 |
| :--- | :--- | :--- |
| Windows 10+ | 稳定主线 | 能力最完整，保持兼容回归 |
| macOS | 主开发线 | 当前优先投入方向，effects / indicator / automation / wasm 持续增强 |
| Linux | 跟随线 | 以编译门禁与合同回归为主 |

> 当前项目节奏是 `macOS mainline first`，同时要求 Windows 行为不回退。

## 快速开始

### Windows

推荐方式：
1. 使用 `Visual Studio 2026` 打开 `MFCMouseEffect.slnx`
2. 默认使用 `Release | x64`
3. 编译后运行 `x64/Release/MFCMouseEffect.exe`

也可以使用仓库封装命令：

```powershell
.\mfx.cmd build
.\mfx.cmd build --shipping
.\mfx.cmd build --gpu
.\mfx.cmd package
```

说明：
- `build`：默认 `Release | x64 | no-gpu`
- `build --shipping`：最小发行构建
- `build --gpu`：打开 Windows GPU hold runtime 构建
- `package`：生成安装包

### macOS

```bash
# 编译并启动 core host
./mfx run

# 跳过 core/WebUI 编译直接运行
./mfx run-no-build

# 指定自动退出秒数，便于快速手测
./mfx run-no-build --seconds 30

# Effects 自检
./mfx effects

# 每日推荐回归
./mfx verify-effects

# 全量 POSIX 回归
./mfx verify-full

# 打包
./mfx package
./mfx package-no-build
```

<details>
<summary>常见问题与重要说明（展开查看）</summary>

## 常见问题与重要说明

这是最建议放进 README 的部分之一，能显著减少“装不上 / 跑不起来 / 看不到效果”的反馈成本。

### 1. macOS 看不到全局输入效果怎么办？

先检查系统权限：
- `Accessibility`
- `Input Monitoring`

当前项目在 macOS 上依赖这两类权限来做全局输入捕获。缺失时，运行时会进入 degraded 状态，但权限恢复后无需强制重启即可恢复。

### 2. Windows 下 `build` 为什么默认不带 GPU？

这是当前项目的明确策略：
- 默认 `--no-gpu`
- 这样更稳，也能减少额外运行时依赖
- 如需 GPU 路线，显式使用 `./mfx build --gpu`

### 3. `Shipping` 构建为什么和普通 `Release` 不完全一样？

`Shipping` 会保留主运行时与 WebUI，但会裁掉一些测试/重诊断能力：
- 不包含 `/api/test` 深度测试路线
- 更适合交付
- 不适合做最深层调试与 proof 类验证

### 4. Linux 是不是已经完整可用？

当前不是主体验平台。

Linux 目前更偏：
- 编译通过
- 合同不漂移
- 跟随主线保持结构一致

如果你要完整体验，优先建议 Windows 或 macOS。

### 5. WebSettings 能改配置，但应用后又恢复了，为什么？

这个项目很多设置项是“前端表单 + 后端真实状态”双向对齐的。  
如果后端绑定没有真正生效，页面刷新后会按真实状态回写，不会假装成功。

这类情况通常优先检查：
- 插件 manifest 路径是否有效
- 对应 lane 是否启用
- 当前平台是否支持该路线
- 是否触发了 fallback

### 6. 插件路线是不是可以任意控制宿主渲染？

不是。

这是项目的关键架构边界：
- 插件负责逻辑计算
- 宿主负责渲染执行、预算校验、回退与资源控制

这个约束是刻意设计的，目的就是让扩展能力和稳定性可以长期共存。

### 7. macOS 打包后第一次打不开怎么办？

这是当前阶段的已知使用说明，不算异常：
- 当前 macOS 包仍是未签名产物
- 第一次启动可能被 Gatekeeper 拦截
- 某些情况下需要在 Finder 中使用 `Open` 完成首次放行
- 当前打包流程还没有接入 notarization

</details>

<details>
<summary>项目结构（展开查看）</summary>

## 项目结构

```text
MFCMouseEffect/
├── MFCMouseEffect/
│   ├── MouseFx/                 # Core: effects, automation, wasm, diagnostics, server
│   ├── Platform/                # Windows / macOS / Linux 平台实现
│   ├── WebUIWorkspace/          # Svelte 设置页源码
│   ├── Runtime/                 # 运行时资源与依赖
│   ├── Assets/                  # Companion / 视觉资源
│   └── WasmRuntimeBridge/       # WASM 运行时桥接
├── tools/
│   ├── platform/regression/     # 回归脚本
│   ├── platform/manual/         # 手测 / 自检脚本
│   └── docs/                    # 文档索引与治理脚本
├── docs/                        # 架构、路线图、问题、回归文档
├── examples/                    # 示例与模板
└── mfx / mfx.cmd                # 推荐命令入口
```

### 结构上的几个亮点

- `MouseFx/Core` 负责语义与能力抽象，不直接把平台细节塞进业务逻辑
- `Platform/windows`、`Platform/macos`、`Platform/linux` 明确拆分平台实现
- `MouseFx/Server` 与 `WebUIWorkspace` 组成统一设置页与 API 服务边界
- `tools/platform/regression` 与 `tools/platform/manual` 让功能具备持续验证能力
- `docs` 不是附属品，而是架构协作的一部分

</details>

<details>
<summary>回归与自检（展开查看）</summary>

## 回归与自检

```bash
# 全量 POSIX 套件
./tools/platform/regression/run-posix-regression-suite.sh --platform auto

# Effects 聚焦回归
./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto

# Automation 合同回归
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto

# WASM 聚焦回归
./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto
```

```bash
# macOS WebSettings 手测
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60

# macOS 特效语义自检
./tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build

# macOS 自动化注入自检
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build

# macOS WASM 运行时自检
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build
```

</details>

<details>
<summary>文档入口（展开查看）</summary>

## 文档入口

- 文档总览：[docs/README.zh-CN.md](./docs/README.zh-CN.md)
- English docs index: [docs/README.md](./docs/README.md)
- 当前高优先级上下文：[docs/agent-context/current.md](./docs/agent-context/current.md)
- macOS 主线快照：[docs/refactoring/phase-roadmap-macos-m1-status.md](./docs/refactoring/phase-roadmap-macos-m1-status.md)
- P2 能力索引：[docs/agent-context/p2-capability-index.md](./docs/agent-context/p2-capability-index.md)

</details>

<details>
<summary>参与贡献（展开查看）</summary>

## 参与贡献

欢迎 Issue、讨论、建议，也欢迎直接参与改进。

在提交较大的功能、架构调整、跨平台行为变更之前，建议先联系确认方向，避免大家在不同假设上重复投入。

推荐方式：
- 先开 [Issue](https://github.com/sqmw/MFCMouseEffect/issues) 说明问题或想法
- 再提交 PR
- 大改动、能力扩张、架构路线讨论，建议先邮件沟通

联系邮箱：
- `ksun22515@gmail.com`

建议贡献方向：
- 新效果与新样式
- WebSettings 体验优化
- WASM 插件样例与工具链
- macOS / Windows 行为对齐
- 文档、测试、自检与回归脚本完善

</details>

## 开源协议

本项目基于 [MIT License](./LICENSE) 开源。

你可以自由使用、修改和分发，但请保留协议与版权声明。

---

<p align="center"><b>觉得有用请点个 <a href="https://github.com/sqmw/MFCMouseEffect">Star ⭐</a>，也欢迎在 Issues/Discussions 留言反馈。</b></p>
