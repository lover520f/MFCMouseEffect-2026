# phase56zzp13: wasm image affine single-source parity (macOS + Windows)

## Context
- Verdict: `Bug/regression risk`.
- Previous behavior:
  - macOS applied part of affine metadata (`affineDx/affineDy`) in `spawn_image_affine`.
  - Windows dropped affine metadata by collapsing command to `base` only.
- Result: cross-platform semantic drift for the same WASM command stream.

## Changes
1. Added Core single-source image command resolver:
- `MouseFx/Core/Wasm/WasmImageCommandConfig.h`
- APIs:
  - `ResolveSpawnImageCommand(const SpawnImageCommandV1&)`
  - `ResolveSpawnImageCommand(const SpawnImageAffineCommandV1&)`

2. Unified affine mapping rules:
- Translation (`affineDx/affineDy`) always applied (preserve existing mac behavior).
- Matrix-derived extras (`scale/rotation`) only applied when `affineEnabled != 0`:
  - scale multiplier from averaged axis norm (`hypot`), clamped to `0.2..5.0`.
  - rotation delta from `atan2(m21, m11)` when finite.

3. Consumed same resolver on both platforms:
- Windows:
  - `MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`
- macOS:
  - `Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Image.cpp`
  - Also removed duplicate request-build logic via shared local `BuildImageOverlayRequest(...)`.

## Validation
1. Core wasm contract:
```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope wasm --build-dir /tmp/mfx-platform-macos-core-automation-build
```

2. Full wasm regression suite:
```bash
./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto --skip-automation-test
```

## Result
- `spawn_image_affine` semantics are now aligned by construction across macOS and Windows command execution paths.
- Future affine behavior tuning is centralized in one Core resolver to prevent platform drift.
