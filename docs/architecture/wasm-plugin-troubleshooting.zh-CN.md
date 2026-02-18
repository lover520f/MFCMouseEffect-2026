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
- 先本地构建 `MFCMouseEffect.slnx`（`x64 Debug/Release`），现在会自动产出 `mfx_wasm_runtime.dll`；
- `mfx_wasm_runtime.dll` 在 `MFCMouseEffect.exe` 同目录，
  或在进程可搜索路径中；
- 桥接导出函数至少包含：
  - `mfx_wasm_runtime_create`
  - `mfx_wasm_runtime_call_on_click`
  - `mfx_wasm_runtime_last_error`

## 3. 插件已加载但看不到效果

现象：
- `wasm.plugin_loaded=true`，但屏幕无可见变化

排查：
- 目前 `spawn_text` / `spawn_image` 已接入可见渲染链路。
- 先看以下诊断字段：
- `wasm.last_output_bytes`
- `wasm.last_command_count`
- `wasm.last_parse_error`
- `wasm.last_rendered_by_wasm`
- `wasm.last_executed_text_commands`
- `wasm.last_executed_image_commands`
- `wasm.last_render_error`
- 如果解析成功但仍无可见效果：
  - 检查当前是否被前台抑制策略（如 VM 前台）影响；
  - 检查 `wasm.last_budget_reason` 与预算截断标记。

## 3.1 `spawn_image` 仍显示内置图形（不是图片文件）

现象：
- 插件调用了 `spawn_image`，但界面显示仍是 `star/ripple` 样式。

排查：
- `plugin.json` 是否声明了 `image_assets`；
- `image_assets` 路径是否相对 `plugin.json` 且文件实际存在；
- 扩展名是否在支持列表内：`.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`；
- `imageId` 是否对应到你期望的资源下标。

说明：
- 图片资源不可解析时，宿主会故意回退到内置渲染器，保证功能不中断。

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

## 6. 插件目录为空（`No plugins discovered`）

现象：
- Web 设置页 WASM 插件目录下拉为空；
- `/api/wasm/catalog` 返回 `count=0`。

排查：
- 查看 `/api/wasm/catalog` 的 `search_roots`，确认实际扫描目录；
- 在 Web 设置页检查“插件扫描路径”是否配置正确（或清空后回退默认扫描目录）；
- 确保这些目录下至少有一个合法 `plugin.json`；
- Debug 从仓库构建目录运行时，宿主会自动扫描 `examples/wasm-plugin-template/dist`；
- 目录为空时，`重载插件` 按钮会被禁用（无活动插件不可重载）。

## 7. 最小自检流程

1. 构建模板（`examples/wasm-plugin-template`）。
2. 复制 `effect.wasm` + `plugin.json` 到插件目录。
3. 调用 `/api/wasm/load-manifest`。
4. 调用 `/api/wasm/enable`。
5. 点击一次，然后查看 `/api/state` 的 `wasm` 诊断字段。

## 8. Web 设置页诊断面板

在 Web 设置 WASM 分区中，重点观察：
- 最近调用指标
- 预算标记
- 预算原因
- 解析错误

若诊断行被告警样式高亮，通常表示预算/解析风险，可能导致无输出或输出受限。
