# WASM Plugin Template Quick Start

This guide is for the official template at:
- `examples/wasm-plugin-template`

## 1. Build template artifact

```bash
cd examples/wasm-plugin-template
npm install
npm run build
```

Or with pnpm:

```bash
cd examples/wasm-plugin-template
pnpm install
pnpm run build
```

Build output:
- `dist/effect.wasm`
- `dist/plugin.json`

Optional sample presets:

```bash
# Build one sample preset
npm run build:sample -- --sample text-burst

# Build all sample presets
npm run build:samples
```

Sample outputs:
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/plugin.json`
- `dist/samples/<sample_key>/assets/*` (if that sample declares `image_assets`)

## 1.1 Optional image assets for `spawn_image`

You can declare plugin image files in `plugin.json`:

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

Rules:
- `image_assets` is optional.
- Each item is a path relative to `plugin.json`.
- Supported extensions: `.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`.
- `imageId` maps to this array index (modulo).
- If assets are missing/invalid, host falls back to built-in image renderers.

## 2. Place plugin into host search path

Create a plugin folder named by `plugin.json.id`:

- Debug path (default): `<exe_dir>/plugins/wasm/<plugin_id>/`
- Release path (default): `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`
- Additional runtime search root: `<exe_dir>/plugins/wasm` (Release fallback for portable/local bundles)
- Debug convenience: if running from repo build output, host also auto-scans `examples/wasm-plugin-template/dist`
  so template artifacts are discoverable without manual copy.
- Web settings supports a configurable extra catalog root (`WASM Plugin -> Catalog root path`).
  Save it to include your custom directory in scan/export flows.

Copy plugin files into that folder:
- always: `effect.wasm` + `plugin.json`
- if `plugin.json.image_assets` exists: copy the referenced `assets/` files together

For sample presets, copy the whole sample output folder (`effect.wasm` + `plugin.json` + `assets/` if present)
from either:
- `dist/` (default template)
- `dist/samples/<sample_key>/` (preset sample)

## 3. Enable plugin from command endpoint

HTTP:

```bash
POST /api/wasm/load-manifest
{
  "manifest_path": "C:\\path\\to\\plugins\\wasm\\demo.click.text-rise.v1\\plugin.json"
}
```

Then:

```bash
POST /api/wasm/enable
```

## 4. ABI contract reminder

The template is aligned to ABI v1:
- `mfx_plugin_get_api_version() -> 1`
- `mfx_plugin_on_click(input_ptr, input_len, output_ptr, output_cap)`
- `mfx_plugin_reset()`

Binary layout is defined in:
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

## 5. Built-in sample preset keys

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

For full template structure and per-sample behavior summary:
- `examples/wasm-plugin-template/README.md`
- `examples/wasm-plugin-template/README.zh-CN.md`

## 6. Troubleshooting

- If `load-manifest` fails, check `entry` in `plugin.json` and file existence.
- Runtime bridge is built from this repo: build `MFCMouseEffect.slnx` (`x64 Debug/Release`) to produce `mfx_wasm_runtime.dll`.
- If runtime bridge is still absent at runtime, host falls back to Null runtime (no command output).
- If output is dropped, check budget diagnostics in `/api/state` `wasm` block.
