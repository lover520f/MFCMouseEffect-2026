# 自定义特效（WASM 路线）

## 目标
定义稳定的 WASM 架构契约：
- 插件只负责“逻辑计算”；
- C++ 宿主负责“渲染执行与资源控制”；
- 先保证行为稳定和回退安全，再做视觉优化。

本文件只保留高价值架构信息；历史阶段过程下沉到 issue 文档。

## 范围
- 包含：
  - 点击文字/图片类自定义逻辑，
  - 宿主预算与回退机制，
  - 设置页策略与诊断，
  - 模板与本地编译流程。
- 不包含：
  - 运行时“任意 JS 直转 WASM”，
  - 插件直接控制窗口/交换链，
  - 可视化节点编辑器。

## 数据流
1. 宿主标准化输入事件。
2. `WasmEffectHost` 调用插件 `on_event`。
3. 插件返回命令缓冲。
4. 宿主做预算校验和命令校验。
5. 宿主执行命令；异常时走回退路径。

核心原则：WASM 计算，C++ 渲染。

## ABI v1 契约
插件导出（前两项必选）：

```c
uint32_t mfx_plugin_get_api_version(void);
uint32_t mfx_plugin_on_event(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);
void mfx_plugin_reset(void); // 推荐实现
```

事件类型遵循宿主标准语义（`click/move/scroll/hold*/hover*`）。

## 命令契约（v1）
当前生产路径支持：
- `spawn_text`
- `spawn_image`

通用字段：
- 变换：`x, y, scale, rotation`
- 运动：`vx, vy, ax, ay`
- 样式：`alpha, color`
- 生命周期：`delay_ms, life_ms`
- 资源：`text_id`/`image_id`

## 预算与回退
默认预算：
- `max_execution_ms <= 1.0`
- `max_commands <= 256`
- `output_buffer_bytes` 受策略约束

回退规则：
- 超时：丢弃本次输出
- 超量：截断命令
- 连续失败：禁用插件路径并回退内置效果

可观测字段：
- `wasm.runtime_backend`
- `wasm.runtime_fallback_reason`

## 当前状态
- 运行时路径、诊断、回退契约已接通。
- 策略字段（`enabled`、`manifest_path`、`fallback_to_builtin_click`、预算）已持久化。
- 模板生态与构建脚本已落在：
  - `examples/wasm-plugin-template`

## 关联文档（权威入口）
- 快速开始：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- 兼容策略：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-compatibility.zh-CN.md`
- 排错手册：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-troubleshooting.zh-CN.md`
- 增量实现记录：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-policy-phase3b.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-budget-policy-phase3c.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-budget-schema-phase3d.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-diagnostics-phase3e.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-state-refresh-phase3f.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-runtime-bridge-self-build.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-plugin-template-sample-presets-phase4.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-plugin-template-full-sample-matrix-phase4b.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-plugin-template-assets-all-formats-phase4c.zh-CN.md`
