# 自定义特效（WASM 路线）架构方案

## 1. 背景与目标

当前项目是 C++ 高性能桌面特效宿主。用户反馈的核心诉求是：
- 可高度自定义点击/文字/图片等特效；
- 不希望牺牲实时性和帧稳定；
- 可由用户自行编写逻辑（借助 AI）并持续迭代。

本方案采用：
- `用户逻辑（WASM） + C++ 宿主渲染（批量命令执行）`

目标：
- 允许用户定义“特效逻辑”；
- 宿主保持“渲染控制权”与“性能预算控制权”；
- 插件编译由用户本地完成（宿主不内置编译器）。

非目标（当前阶段）：
- 不支持“任意 JS 直接运行即 WASM”；
- 不开放插件直接控制 GPU 交换链/窗口合成；
- 不在第一阶段做可视化节点编辑器（Render Graph Editor）。

## 2. 总体架构

### 2.1 分层
1. `Event Capture Layer (C++)`
- 捕获点击/滚轮/长按/手势事件，标准化事件参数。

2. `Wasm Effect Host (C++)`
- 加载插件 `.wasm`；
- 以固定 ABI 调用插件入口；
- 接收插件输出的命令缓冲区；
- 执行预算与超限裁剪。

3. `Render Execution Layer (C++)`
- 把命令缓冲区转为宿主内部可渲染对象；
- 走现有批渲染与生命周期系统。

### 2.2 核心原则
- WASM 只“算逻辑/生成命令”，不直接渲染；
- 跨边界调用按“事件级批量”进行，避免高频小调用；
- 插件能力由 ABI 版本管理，保持向后兼容策略。

## 3. 插件模型（用户视角）

### 3.1 用户工作流
1. 基于模板编写插件（建议 AssemblyScript 风格）；
2. 使用文档脚本在本地编译生成 `.wasm`；
3. 在设置页选择并启用该插件；
4. 触发事件时由宿主调用插件并执行返回命令。

当前官方模板位置：
- `examples/wasm-plugin-template`
- 快速开始：`docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- 兼容策略：`docs/architecture/wasm-plugin-compatibility.zh-CN.md`
- 排错手册：`docs/architecture/wasm-plugin-troubleshooting.zh-CN.md`

### 3.2 插件交付物
- `effect.wasm`
- `plugin.json`（清单，包含名字、版本、ABI 版本、入口能力）

示例 `plugin.json`：

```json
{
  "id": "demo.click.emojis",
  "name": "Demo Emoji Click",
  "version": "0.1.0",
  "api_version": 1,
  "entry": "effect.wasm"
}
```

## 4. ABI（v1）草案

为保证跨语言稳定，ABI 使用 C 风格导出。

```c
// 必选：返回插件 ABI 版本
uint32_t mfx_plugin_get_api_version(void);

// 推荐：统一事件入口
// kind 支持 click/move/scroll/hold-start/hold-update/hold-end/hover-start/hover-end
uint32_t mfx_plugin_on_event(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);

// 可选：插件重置（热重载或设置变化时）
void mfx_plugin_reset(void);
```

兼容规则：
- 插件必须导出 `mfx_plugin_get_api_version` 与 `mfx_plugin_on_event`。

输入与输出使用二进制结构（或紧凑 JSON，MVP 先二进制更省开销）。

## 5. 命令缓冲区（v1）草案

MVP 先支持两类发射命令：
- `spawn_text`
- `spawn_image`

统一字段建议：
- 位置：`x, y`
- 运动：`vx, vy, ax, ay`
- 样式：`scale, rotation, alpha, color`
- 生命周期：`delay_ms, life_ms`
- 资源：`text_id` 或 `image_id`

宿主负责：
- 资源 ID 解析（字体/图片）；
- 实际对象创建与渲染；
- 超限裁剪（例如超过上限的命令丢弃）。

## 6. 性能预算（建议初值）

MVP 默认预算：
- 单次事件插件执行时间：`<= 1.0 ms`
- 单次事件最大命令数：`<= 256`
- 单插件线性内存上限：`<= 4 MB`
- 单帧最大新建对象数：`<= 512`

处理策略：
- 超时：本次事件命令清空并记录诊断；
- 超命令数：截断尾部命令；
- 内存超限：禁用该插件并回退到内置效果。
- 运行时桥接回退可观测性：查看 `wasm.runtime_backend` 与 `wasm.runtime_fallback_reason`。

## 7. 打包体积与运行时选择

建议运行时：
- 轻量运行时优先（如 WAMR/wasm3 级别）

预估增量（目标）：
- 运行时 + 宿主桥接 + 基础模板文档：约 `1~3 MB`

说明：
- 编译工具链不随安装包分发（用户自行安装），可显著控制安装包体积。

## 8. 分阶段实施计划

### Phase 1（先做闭环，不改渲染主链）
- 新增 `WasmEffectHost` 骨架；
- 定义 ABI v1 与输入输出结构；
- 点击事件可调用插件并产生日志化命令（先不落渲染）。

验收：
- 能加载样例插件；
- 点击触发后可看到解析后的命令日志；
- 主流程稳定无崩溃。

### Phase 2（命令执行接入）
- 命令缓冲区接入现有点击文字/图片渲染入口；
- 完成预算控制和超限处理；
- 增加运行时诊断字段到状态页。

验收：
- 样例插件可在屏幕产生可见效果；
- 超限时有可观测诊断，不影响宿主主循环。

### Phase 3（Web 设置与插件管理）
- Web 设置页增加插件选择、启停、重载、诊断信息；
- 支持插件目录扫描与清单校验；
- 增加“回退到内置特效”策略开关。

验收：
- 可在设置页无重启切换插件；
- 插件异常可自动回退内置特效。

### Phase 4（生态与模板）
- 发布官方模板仓库（AssemblyScript）；
- 提供编译脚本示例与常见效果模板；
- 完善 API 文档与版本兼容说明。

## 9. 风险与对策

1. ABI 早期频繁变更
- 对策：从 v1 起版本化，新增字段只追加，不破坏旧布局。

2. 用户插件性能不可控
- 对策：严格预算 + 截断 + 失败回退。

3. 调试门槛
- 对策：提供最小模板、固定日志格式、错误码文档。

## 10. 当前决策结论

采用：
- `WASM 逻辑插件 + C++ 宿主渲染`

暂不采用：
- 纯 WebView 作为主渲染链；
- 直接开放原生 DLL 插件（面向普通用户阶段）。

该方案用于后续实现阶段的评审基线文档。

## 11. 落地进度（15 提交计划）

当前按“小步快跑”推进，先搭建架构骨架，再逐步接入事件链和设置页。

- [x] Commit 1：`MouseFx/Core/Wasm` 模块骨架（Host/Runtime/ABI）与工程接线
- [x] Commit 2：ABI v1 输入/输出序列化与命令缓冲解析器
- [x] Commit 3：插件清单模型（`plugin.json`）和校验器
- [x] Commit 4：插件目录发现与路径策略（默认目录 + 配置目录）
- [x] Commit 5：WasmEffectHost 生命周期（load/unload/reload）完善
- [x] Commit 6：`AppController` 启停链路接入（仅初始化，不改行为）
- [x] Commit 7：点击事件链路调用 Host（先日志化，不接渲染）
- [x] Commit 8：预算控制（耗时/输出大小/命令数）和诊断状态
- [x] Commit 9：诊断状态映射到 Settings 状态输出
- [x] Commit 10：Web API 暴露只读 WASM 状态
- [x] Commit 11：插件启停/重载命令入口（IPC/HTTP）
- [x] Commit 12：RuntimeFactory 扩展到真实 WASM 运行时（保留 Null 兜底）
- [x] Commit 13：官方模板与本地编译脚本样例
- [x] Commit 14：文档完善（快速开始/排错/兼容策略）
- [x] Commit 15：回归与稳定性收口（默认关闭、异常回退验证）

## 12. 真正落地进度（5 提交计划）

说明：15 提交完成的是“架构基线”，以下 5 提交用于把 WASM 命令接入真实可见渲染链。

- [x] Commit A1：`spawn_text` 命令执行器落地到点击链路（命中后优先渲染 WASM 文本）
- [x] Commit A2：`spawn_image` 命令执行器落地到点击链路（命中后渲染图片/图标指令）
- [x] Commit A3：`text_id/image_id` 资源映射层与缺省回退策略（确定性映射：文本池 + 渲染器池）
- [x] Commit A4：渲染执行与预算/异常联动收口（失败不影响主流程）
- [x] Commit A5：端到端回归与文档验收步骤固化

## 13. 增量进度（Phase 3 Web 管理面板）

- [x] Phase 3a：WASM Web 设置面板接线
  - 新增 `/api/wasm/catalog`
  - 新增 Svelte 分区（目录、加载、启停、重载、诊断展示）
  - 新增 i18n、构建链路与安装包预检（`wasm-settings.svelte.js`）
- [x] Phase 3b：设置页策略控制（回退开关/配置级行为）
  - 新增持久化 `wasm` 配置（`enabled`、`fallback_to_builtin_click`、`manifest_path`）
  - 新增 `/api/wasm/policy` 与 `wasm_set_policy` 命令
  - 启动流程支持清单路径与启用状态恢复
  - 点击分发链路支持“WASM 激活时是否回退内置点击特效”策略
  - 详见：`docs/issues/wasm-web-settings-policy-phase3b.zh-CN.md`
- [x] Phase 3c：可配置 WASM 执行预算策略
  - 新增持久化预算策略（`output_buffer_bytes`、`max_commands`、`max_execution_ms`）
  - 启动/重载/策略更新时，Host 预算从配置同步
  - Web 面板支持预算编辑与运行时预算快照展示
  - 详见：`docs/issues/wasm-web-settings-budget-policy-phase3c.zh-CN.md`
- [x] Phase 3d：预算输入 schema 驱动与默认恢复
  - Web 面板去除硬编码预算范围（`min/max/step` 来自 schema）
  - 新增共享策略模型，统一范围归一化与 clamp/snap
  - 新增“恢复默认策略”动作，并与服务端默认范围对齐
  - 详见：`docs/issues/wasm-web-settings-budget-schema-phase3d.zh-CN.md`
- [x] Phase 3e：运行时诊断可视化
  - WASM 面板新增预算遥测展示（调用指标/预算标记/预算原因/解析错误）
  - 抽取诊断模型，统一归一化与风险高亮判定
  - 新增诊断告警样式与中英文文案
  - 详见：`docs/issues/wasm-web-settings-diagnostics-phase3e.zh-CN.md`
- [x] Phase 3f：WASM 动作后的状态优先刷新
  - WASM 动作由“每次全量 reload”改为“状态优先刷新”
  - 语言未变化时复用缓存 schema，减少重复请求
  - 局部刷新失败时自动回退全量 reload，保证稳定性
  - 详见：`docs/issues/wasm-web-settings-state-refresh-phase3f.zh-CN.md`

## 14. 增量进度（Phase 4 模板生态）

- [x] Phase 4a：官方样例预设矩阵与构建脚本拆分
  - 新增 AssemblyScript 公共模块（`common/abi.ts`、`common/random.ts`）
  - 新增样例预设（`text-rise`、`text-burst`、`image-pulse`、`mixed-text-image`）
  - 新增脚本工具链（`build-lib`、`build-sample`、`build-all-samples`）
  - 新增 pnpm 兼容编译器探测（兼容 `asc.js` 与旧 `asc` 路径）
  - 详见：`docs/issues/wasm-plugin-template-sample-presets-phase4.zh-CN.md`
- [x] Phase 4b：面向用户的结构文档与全量样例扩充
  - 样例扩展到完整矩阵（`text-spiral`、`text-wave-chain`、`image-burst`、`image-lift`、`mixed-emoji-celebrate`、`button-adaptive`）
  - 样例构建脚本支持按预设输出 `image_assets` 清单字段
  - 新增模板中英文文档（`examples/wasm-plugin-template/README.md`、`examples/wasm-plugin-template/README.zh-CN.md`）
  - 详见：`docs/issues/wasm-plugin-template-full-sample-matrix-phase4b.zh-CN.md`
- [x] Phase 4c：真实图片资源包与全格式样例产物
  - 模板资源包覆盖全部支持格式（`png/jpg/jpeg/bmp/gif/tif/tiff`）
  - 样例构建脚本自动复制 `image_assets` 到产物目录
  - 预设清单映射调整为覆盖全部格式
  - 详见：`docs/issues/wasm-plugin-template-assets-all-formats-phase4c.zh-CN.md`

## 15. Runtime Bridge 自构建状态

- [x] `mfx_wasm_runtime.dll` 已纳入仓库工程，可本地直接构建。
- 构建接线：
- `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj`
- `MFCMouseEffect.slnx` 与 `MFCMouseEffect/MFCMouseEffect.vcxproj` 已建立依赖
- 安装包接线：
- `Install/MFCMouseEffect.iss` 增加 `mfx_wasm_runtime.dll` 预检与打包
- 详见：`docs/issues/wasm-runtime-bridge-self-build.zh-CN.md`
