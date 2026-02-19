# WASM Template Phase4b: Full Sample Matrix and Bilingual User Docs

## Context

Users need to understand the `wasm-example` structure quickly and start from complete references.
The previous template docs were partial:
- only English README in template root,
- sample key list did not fully match the actual sample preset matrix,
- some preset references were missing source files.

## Changes

## 1. Sample matrix completion

Added missing sample implementations under:
- `examples/wasm-plugin-template/assembly/samples/mixed-emoji-celebrate.ts`
- `examples/wasm-plugin-template/assembly/samples/button-adaptive.ts`

Now the preset matrix is complete and buildable:
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

## 2. Sample manifest image_assets emission

Updated scripts so sample manifests can carry preset image assets:
- `examples/wasm-plugin-template/scripts/build-sample.mjs`
- `examples/wasm-plugin-template/scripts/build-all-samples.mjs`
- `examples/wasm-plugin-template/scripts/sample-presets.mjs`

Behavior:
- if a preset defines `imageAssets`, generated `plugin.json` now contains `image_assets`.

## 3. User-facing structure docs (EN + ZH)

Updated and expanded:
- `examples/wasm-plugin-template/README.md`

Added Chinese counterpart:
- `examples/wasm-plugin-template/README.zh-CN.md`

Both now include:
- directory structure,
- full sample matrix,
- build commands,
- manifest rules,
- runtime placement.

## 4. Architecture/quickstart sync

Updated:
- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- `docs/architecture/custom-effects-wasm-route.md`
- `docs/architecture/custom-effects-wasm-route.zh-CN.md`

## Validation

Run in `examples/wasm-plugin-template`:

```bash
pnpm run build:samples
```

Expected:
- all sample keys compile successfully,
- each sample output folder contains `effect.wasm` and `plugin.json`,
- presets with assets contain `image_assets` in `plugin.json`.

## Result

The template is now user-ready as a reference pack:
- complete sample coverage,
- bilingual root docs,
- aligned quickstart and architecture references.
