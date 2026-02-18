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

## 2. Place plugin into host search path

Create a plugin folder named by `plugin.json.id`:

- Debug path (default): `<exe_dir>/plugins/wasm/<plugin_id>/`
- Release path (default): `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

Copy `dist/effect.wasm` and `dist/plugin.json` into that folder.

## 3. Enable plugin from command endpoint

HTTP:

```bash
POST /api/wasm/load-manifest
{
  "manifest_path": "C:\\path\\to\\plugins\\wasm\\demo.click.text.v1\\plugin.json"
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

## 5. Troubleshooting

- If `load-manifest` fails, check `entry` in `plugin.json` and file existence.
- If runtime bridge is absent, host falls back to Null runtime (no command output).
- If output is dropped, check budget diagnostics in `/api/state` `wasm` block.
