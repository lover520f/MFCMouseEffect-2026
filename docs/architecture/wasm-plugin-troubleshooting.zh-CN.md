# WASM 插件排错手册

本手册对应当前 v1 路线：
- 运行时桥接：`mfx_wasm_runtime.dll`
- 宿主诊断：`/api/state` -> `wasm`

## 1. Manifest 加载失败

现象：
- `POST /api/wasm/load-manifest` 返回 `ok=false`
- `wasm.last_error` 提示 manifest 或 wasm 路径问题

排查：
- `manifest_path` 使用绝对路径且文件存在；
- `plugin.json` 字段完整；
- `entry` 指向的 `effect.wasm` 文件存在。

## 2. 运行时桥接加载失败

现象：
- `wasm.plugin_loaded=false`
- `wasm.last_error` 含 dll/export 相关报错
- `wasm.runtime_backend="null"` 且 `wasm.runtime_fallback_reason` 非空

排查：
- `mfx_wasm_runtime.dll` 在 `MFCMouseEffect.exe` 同目录，
  或在进程可搜索路径中；
- 桥接导出函数至少包含：
  - `mfx_wasm_runtime_create`
  - `mfx_wasm_runtime_call_on_click`
  - `mfx_wasm_runtime_last_error`

## 3. 插件已加载但看不到效果

现象：
- `wasm.plugin_loaded=true`，但屏幕无可见变化

说明：
- 当前阶段点击链路先做“命令解析 + 诊断”闭环，
  自定义命令到最终渲染主链还在后续阶段接入。

此时可先看：
- `wasm.last_output_bytes`
- `wasm.last_command_count`
- `wasm.last_parse_error`

## 4. 预算拒绝或截断

现象：
- `wasm.last_call_rejected_by_budget=true`
- `wasm.last_output_truncated_by_budget=true`
- `wasm.last_command_truncated_by_budget=true`

原因字段：
- `wasm.last_budget_reason`

建议：
- 降低单次事件命令数；
- 减少输出字节；
- 简化单次调用计算成本。

## 5. 解析错误

`wasm.last_parse_error` 常见值：
- `truncated_header`
- `invalid_command_size`
- `truncated_command`
- `unsupported_command_kind`
- `command_limit_exceeded`

建议：
- 严格对齐 `WasmPluginAbi.h` 的二进制布局；
- `sizeBytes` 必须与实际写入的结构体字节数一致。

## 6. 最小自检流程

1. 构建模板（`examples/wasm-plugin-template`）。
2. 复制 `effect.wasm` + `plugin.json` 到插件目录。
3. 调用 `/api/wasm/load-manifest`。
4. 调用 `/api/wasm/enable`。
5. 点击一次，然后查看 `/api/state` 的 `wasm` 诊断字段。
