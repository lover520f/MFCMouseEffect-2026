# MFCMouseEffect WASM Plugin Template

Official AssemblyScript template for `MFCMouseEffect` WASM v1 plugins.

## Outputs

Default build (`npm run build`) generates:
- `dist/effect.wasm`
- `dist/effect.wat`
- `dist/plugin.json`

## Install and Build

```bash
npm install
npm run build
```

Or with pnpm:

```bash
pnpm install
pnpm run build
```

## Sample Presets

Template now includes multiple sample entries:
- `text-rise`
- `text-burst`
- `image-pulse`
- `mixed-text-image`

Build one sample:

```bash
npm run build:sample -- --sample text-burst
```

Build all sample bundles:

```bash
npm run build:samples
```

Sample outputs are under:
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/plugin.json`

## Runtime Location

Copy `effect.wasm` + `plugin.json` to:
- Debug: `<exe_dir>/plugins/wasm/<plugin_id>/`
- Release: `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

`<plugin_id>` must match `plugin.json.id`.

## ABI Reminder

Current ABI version:
- `api_version = 1`

Required exports:
- `mfx_plugin_get_api_version`
- `mfx_plugin_on_click`
- `mfx_plugin_reset`

Binary layout source of truth:
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

## More Docs

- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- `docs/architecture/wasm-plugin-compatibility.md`
- `docs/architecture/wasm-plugin-troubleshooting.md`
