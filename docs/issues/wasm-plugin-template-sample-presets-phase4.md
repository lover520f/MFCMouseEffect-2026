# WASM Template Phase4: Sample Presets and Build Tooling

## Context

Phase4 requires an official template that users can compile locally with minimal friction.
The previous template had only one entry and one build command, which made it hard to:
- share reusable examples,
- compare effect styles quickly,
- debug AssemblyScript compile issues in isolation.

## Changes

## 1. Template code split

- Added shared modules:
  - `examples/wasm-plugin-template/assembly/common/abi.ts`
  - `examples/wasm-plugin-template/assembly/common/random.ts`
- Added sample preset entries:
  - `examples/wasm-plugin-template/assembly/samples/text-rise.ts`
  - `examples/wasm-plugin-template/assembly/samples/text-burst.ts`
  - `examples/wasm-plugin-template/assembly/samples/image-pulse.ts`
  - `examples/wasm-plugin-template/assembly/samples/mixed-text-image.ts`
- Kept `assembly/index.ts` as default entry by re-exporting `text-rise`.

## 2. Build tooling refactor

- Added shared build helpers:
  - `examples/wasm-plugin-template/scripts/build-lib.mjs`
  - `examples/wasm-plugin-template/scripts/sample-presets.mjs`
- Added sample build commands:
  - `examples/wasm-plugin-template/scripts/build-sample.mjs`
  - `examples/wasm-plugin-template/scripts/build-all-samples.mjs`
- Updated:
  - `examples/wasm-plugin-template/scripts/build.mjs`
  - `examples/wasm-plugin-template/package.json`

New commands:
- `npm run build`
- `npm run build:sample -- --sample <key>`
- `npm run build:samples`

## 3. pnpm compatibility

`build-lib.mjs` now resolves compiler paths for both cases:
- `node_modules/assemblyscript/bin/asc.js` (pnpm/common)
- `node_modules/assemblyscript/bin/asc` (legacy path)

This prevents false "compiler not found" errors.

## 4. Template metadata and docs sync

- Updated default manifest:
  - `examples/wasm-plugin-template/plugin.json`
  - default `id`: `demo.click.text-rise.v1`
- Updated docs:
  - `examples/wasm-plugin-template/README.md`
  - `docs/architecture/wasm-plugin-template-quickstart.md`
  - `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
  - `docs/architecture/wasm-plugin-troubleshooting.md`
  - `docs/architecture/wasm-plugin-troubleshooting.zh-CN.md`

## Validation

Executed in `examples/wasm-plugin-template`:

```bash
node ./scripts/build.mjs
node ./scripts/build-sample.mjs --sample mixed-text-image
node ./scripts/build-all-samples.mjs
```

Expected result:
- default bundle in `dist/`
- per-sample bundles in `dist/samples/<sample_key>/`
- no AssemblyScript type conversion errors.

## Notes

During validation, `text-burst` had `f64 -> f32` conversion errors.
Fixed by explicit `f32` typing/casts in `assembly/samples/text-burst.ts`.
