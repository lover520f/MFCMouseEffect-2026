# WASM 插件模板快速开始

本指南对应官方模板目录：
- `examples/wasm-plugin-template`

## 1. 构建模板产物

```bash
cd examples/wasm-plugin-template
npm install
npm run build
```

或使用 pnpm：

```bash
cd examples/wasm-plugin-template
pnpm install
pnpm run build
```

构建产物：
- `dist/effect.wasm`
- `dist/plugin.json`

可选的样例预设构建：

```bash
# 构建单个样例预设
npm run build:sample -- --sample text-burst

# 一次构建全部样例预设
npm run build:samples
```

样例产物位置：
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/plugin.json`

## 2. 放到宿主插件目录

按 `plugin.json.id` 建目录：

- Debug 默认目录：`<exe_dir>/plugins/wasm/<plugin_id>/`
- Release 默认目录：`%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

把 `effect.wasm` 和 `plugin.json` 复制到该目录，来源可为：
- `dist/`（默认模板构建）
- `dist/samples/<sample_key>/`（样例预设构建）

## 3. 通过命令接口加载并启用

HTTP：

```bash
POST /api/wasm/load-manifest
{
  "manifest_path": "C:\\path\\to\\plugins\\wasm\\demo.click.text-rise.v1\\plugin.json"
}
```

然后：

```bash
POST /api/wasm/enable
```

## 4. ABI 契约提醒

模板与 ABI v1 对齐：
- `mfx_plugin_get_api_version() -> 1`
- `mfx_plugin_on_click(input_ptr, input_len, output_ptr, output_cap)`
- `mfx_plugin_reset()`

二进制布局定义在：
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

## 5. 内置样例预设 key

- `text-rise`
- `text-burst`
- `image-pulse`
- `mixed-text-image`

## 6. 排错

- `load-manifest` 失败：先检查 `plugin.json` 的 `entry` 与文件是否存在。
- 运行时桥接库由本仓库构建：先编译 `MFCMouseEffect.slnx`（`x64 Debug/Release`）生成 `mfx_wasm_runtime.dll`。
- 若运行时仍找不到桥接库，宿主会回退 Null runtime（不会产生命令输出）。
- 触发后无效果时，先看 `/api/state` 里的 `wasm` 诊断字段是否被预算裁剪。
