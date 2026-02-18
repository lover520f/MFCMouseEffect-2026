# MFCMouseEffect WASM Plugin Template

This template generates:
- `dist/effect.wasm`
- `dist/plugin.json`

## Build

```bash
npm install
npm run build
```

Or with pnpm:

```bash
pnpm install
pnpm run build
```

## Runtime location

Copy the two files from `dist/` to:
- Debug: `<exe_dir>/plugins/wasm/<plugin_id>/`
- Release: `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

`<plugin_id>` should match `plugin.json` `id`.

For full steps, see:
- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
