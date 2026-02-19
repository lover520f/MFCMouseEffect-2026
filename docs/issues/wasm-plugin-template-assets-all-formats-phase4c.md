# WASM Template Phase4c: Real Asset Bundle and Full Format Coverage

## Context

The template had `image_assets` wiring but sample outputs were not self-contained:
- assets were not copied into `dist/samples/...`,
- users could import sample manifests but still miss image files,
- format coverage examples were not explicit.

## Changes

## 1. Real asset bundle for template

Added `examples/wasm-plugin-template/assets` with real downloaded/generated files.
Covered all host-supported formats in template examples:
- `.png`
- `.jpg`
- `.jpeg`
- `.bmp`
- `.gif`
- `.tif`
- `.tiff`

## 2. Preset mapping updated to use full format set

Updated:
- `examples/wasm-plugin-template/scripts/sample-presets.mjs`

Notable mappings:
- `image-burst` now includes `emoji-1.jpeg`
- `image-lift` now uses `lift-a.tif` + `lift-b.tiff`
- `button-adaptive` now uses `btn-middle.tiff`

## 3. Build scripts now copy referenced assets

Updated:
- `examples/wasm-plugin-template/scripts/build-lib.mjs`
- `examples/wasm-plugin-template/scripts/build.mjs`
- `examples/wasm-plugin-template/scripts/build-sample.mjs`
- `examples/wasm-plugin-template/scripts/build-all-samples.mjs`

Added shared helper:
- `copyRelativeFiles(rootDir, outputDir, relativeFiles)`

Behavior:
- when manifest or sample preset declares `image_assets`, build output now includes copied files under corresponding `assets/` paths.

## 4. User docs sync (EN + ZH)

Updated:
- `examples/wasm-plugin-template/README.md`
- `examples/wasm-plugin-template/README.zh-CN.md`
- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`

## Validation

Run in template directory:

```bash
pnpm run build:samples
```

Expected:
- all sample presets build successfully,
- sample manifests include correct `image_assets`,
- output folders contain corresponding `assets/*` files.

## Outcome

Template sample outputs are now directly importable and runnable without manual asset patching.
