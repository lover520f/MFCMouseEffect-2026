# WASM Image Assets Support (GIF + Static Images) (2026-02)

## Goal
- Make `spawn_image` support plugin-provided image files, not only built-in renderers.
- Supported file types:
  - `png`
  - `jpg/jpeg`
  - `bmp`
  - `gif` (animated frames)
  - `tif/tiff`

## Manifest
`plugin.json` now supports optional `image_assets`:

```json
{
  "id": "demo.click.image-pack.v1",
  "name": "Demo Click Image Pack",
  "version": "0.1.0",
  "api_version": 1,
  "entry": "effect.wasm",
  "image_assets": [
    "assets/smile.png",
    "assets/cat.gif",
    "assets/coin.jpg"
  ]
}
```

Notes:
- Paths are relative to `plugin.json`.
- `imageId` maps to array index (modulo when out of range).
- If assets are missing/invalid, runtime falls back to built-in image renderers.

## Implementation
1. Manifest model
- `WasmPluginManifest` parses and validates `image_assets`.
- Validation includes: relative path only, no parent traversal, allowed extensions.

2. Asset catalog cache
- Added `WasmPluginImageAssetCatalog`:
  - caches per `manifestPath` with manifest write-time checks;
  - resolves absolute image path by `imageId`.

3. File renderer
- Added `WasmImageFileRenderer` (`IRippleRenderer`):
  - renders static images;
  - reads GIF frame delays and advances by elapsed time;
  - supports alpha and optional tint matrix.

4. Execution path
- `DispatchRouter -> WasmClickCommandExecutor` now passes `activeManifestPath`.
- `WasmRenderResourceResolver::CreateImageRendererById` tries plugin image asset first, then falls back to built-ins.

