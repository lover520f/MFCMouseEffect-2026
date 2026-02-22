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
- `dist/samples/<sample_key>/assets/*`（该样例声明了 `image_assets` 时）

## 1.1 `spawn_image` 的可选图片资源

可在 `plugin.json` 中声明插件图片资源：

```json
{
  "id": "demo.click.image-pack.v1",
  "name": "Demo Click Image Pack",
  "version": "0.1.0",
  "api_version": 1,
  "entry": "effect.wasm",
  "image_assets": [
    "assets/smile.png",
    "assets/cat.gif"
  ]
}
```

规则：
- `image_assets` 为可选字段；
- 每一项路径相对 `plugin.json`；
- 支持扩展名：`.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`；
- `imageId` 按数组下标映射（越界取模）；
- 图片缺失或无效时，宿主会自动回退到内置图片渲染器。

## 2. 放到宿主插件目录

按 `plugin.json.id` 建目录：

- Debug 默认目录：`<exe_dir>/plugins/wasm/<plugin_id>/`
- Release 默认目录：`%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`
- 额外扫描目录：`<exe_dir>/plugins/wasm`（Release 便携/本地打包兜底）
- Debug 便捷能力：从仓库构建目录运行时，宿主会额外扫描 `examples/wasm-plugin-template/dist`，
  模板产物可直接被发现（无需先手工复制）。
- Web 设置页支持配置“额外扫描目录”（`WASM 插件 -> 插件扫描路径`），
  保存后会进入插件扫描与“导出全部插件”的目录集合。

把插件文件复制到该目录：
- 必选：`effect.wasm` + `plugin.json`
- 若 `plugin.json.image_assets` 存在：需连同 `assets/` 一起复制

样例预设建议直接复制整个样例目录（`effect.wasm` + `plugin.json` + `assets/`），来源可为：
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
- `mfx_plugin_reset()`

事件入口导出：
- `mfx_plugin_on_event(input_ptr, input_len, output_ptr, output_cap)`（必需）

当前模板样例：
- 内置样例预设已全部导出 `mfx_plugin_on_event`。

兼容规则：
- 插件必须导出 `mfx_plugin_on_event`。

二进制布局定义在：
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

## 5. 内置样例预设 key

- `text-rise`
- `text-burst`
- `text-spiral`
- `text-wave-chain`
- `image-pulse`
- `image-burst`
- `image-lift`
- `mixed-text-image`
- `mixed-emoji-celebrate`
- `button-adaptive`
- `scroll-particle-burst`

模板目录结构与每个样例的行为说明见：
- `examples/wasm-plugin-template/README.md`
- `examples/wasm-plugin-template/README.zh-CN.md`

## 6. 排错

- `load-manifest` 失败：先检查 `plugin.json` 的 `entry` 与文件是否存在。
- 运行时桥接库由本仓库构建：先编译 `MFCMouseEffect.slnx`（`x64 Debug/Release`）生成 `mfx_wasm_runtime.dll`。
- 若运行时仍找不到桥接库，宿主会回退 Null runtime（不会产生命令输出）。
- 触发后无效果时，先看 `/api/state` 里的 `wasm` 诊断字段是否被预算裁剪。
