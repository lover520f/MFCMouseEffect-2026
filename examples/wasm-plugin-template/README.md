# MFCMouseEffect WASM Plugin Template

Official AssemblyScript template for `MFCMouseEffect` WASM v1 plugins.

Language: [English](README.md) | [中文](README.zh-CN.md)

## What This Template Contains

- Stable ABI helpers for click/event input and command-buffer output.
- Reusable random/color helpers.
- A full sample matrix covering text, image, mixed, button-adaptive, and scroll-event effects.
- Build scripts for default build, single sample build, and all-samples build.

## Directory Structure

```text
examples/wasm-plugin-template/
  assembly/
    common/
      abi.ts                  # ABI constants + read/write helpers
      random.ts               # deterministic pseudo-random helpers
    samples/
      text-rise.ts
      text-burst.ts
      text-spiral.ts
      text-wave-chain.ts
      image-pulse.ts
      image-burst.ts
      image-lift.ts
      mixed-text-image.ts
      mixed-emoji-celebrate.ts
      button-adaptive.ts
      scroll-particle-burst.ts
    index.ts                  # default entry (currently exports text-rise)
  scripts/
    build-lib.mjs             # shared build helper
    sample-presets.mjs        # sample matrix metadata
    build.mjs                 # build default entry
    build-sample.mjs          # build one sample by key
    build-all-samples.mjs     # build all sample bundles
    clean.mjs                 # clean dist
  plugin.json                 # default manifest template
  asconfig.json
  package.json
  README.md
  README.zh-CN.md
```

## Install and Build

```bash
pnpm install
pnpm run build
```

Or with npm:

```bash
npm install
npm run build
```

Default output:
- `dist/effect.wasm`
- `dist/effect.wat`
- `dist/plugin.json`

## Build Sample Presets

Build one sample:

```bash
pnpm run build:sample -- --sample text-burst
```

Build all samples:

```bash
pnpm run build:samples
```

Sample output layout:
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/effect.wat`
- `dist/samples/<sample_key>/plugin.json`
- `dist/samples/<sample_key>/assets/*` (auto-copied image assets referenced by `image_assets`)

## Full Sample Matrix

| key | category | behavior summary | image_assets |
| --- | --- | --- | --- |
| `text-rise` | text | single upward text with button-sensitive drift | no |
| `text-burst` | text | dual-side text burst | no |
| `text-spiral` | text | tri-text spiral-like spread | no |
| `text-wave-chain` | text | 4-step waved text chain | no |
| `image-pulse` | image | single pulse image chosen by button | yes |
| `image-burst` | image | 3-image radial burst | yes |
| `image-lift` | image | 2-image lift sequence | yes |
| `mixed-text-image` | mixed | one text + one image | yes |
| `mixed-emoji-celebrate` | mixed | 2 texts + 2 images celebration | yes |
| `button-adaptive` | mixed | text/image ids adapt by mouse button | yes |
| `scroll-particle-burst` | event(scroll) | wheel-driven colorful particle burst with ring accents | yes |

`sample-presets.mjs` is the source of truth for:
- sample key
- source entry file
- plugin id/name/version
- optional `image_assets`

## Built-in Asset Pack (all supported formats)

Template `assets/` now includes and uses all host-supported image formats:
- `.png`: `smile.png`, `confetti.png`, `crown.png`, `emoji-2.png`, `mix-a.png`, `btn-left.png`
- `.jpg`: `coin.jpg`, `mix-b.jpg`
- `.jpeg`: `emoji-1.jpeg`
- `.bmp`: `star.bmp`
- `.gif`: `cat.gif`, `emoji-3.gif`, `party.gif`, `btn-right.gif`
- `.tif`: `lift-a.tif`
- `.tiff`: `lift-b.tiff`, `btn-middle.tiff`

These files are downloaded/generated into `examples/wasm-plugin-template/assets`
and copied into sample build outputs automatically.

## Manifest (`plugin.json`)

Minimal fields:

```json
{
  "id": "demo.click.text-rise.v1",
  "name": "Demo Click Text Rise",
  "version": "0.1.0",
  "api_version": 1,
  "entry": "effect.wasm"
}
```

Optional image file mapping:

```json
{
  "image_assets": [
    "assets/smile.png",
    "assets/cat.gif"
  ]
}
```

Rules:
- paths are relative to `plugin.json`
- supported formats: `.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`
- `imageId` maps to array index (modulo)
- invalid/missing assets fallback to built-in host renderers

## Runtime Placement

Copy `effect.wasm` + `plugin.json` to one plugin folder:
- Debug: `<exe_dir>/plugins/wasm/<plugin_id>/`
- Release: `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

`<plugin_id>` must equal `plugin.json.id`.

For sample bundles, copy the whole sample folder (`effect.wasm` + `plugin.json` + `assets/`).

## ABI Reminder

Current ABI:
- `api_version = 1`

Required exports:
- `mfx_plugin_get_api_version`
- `mfx_plugin_reset`

Event entry exports:
- `mfx_plugin_on_event` (required by current host; supports click/move/scroll/hold/hover)

Host compatibility rule:
- plugin must export `mfx_plugin_on_event`.

Binary layout source of truth:
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

## Related Docs

- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- `docs/architecture/wasm-plugin-compatibility.md`
- `docs/architecture/wasm-plugin-troubleshooting.md`
