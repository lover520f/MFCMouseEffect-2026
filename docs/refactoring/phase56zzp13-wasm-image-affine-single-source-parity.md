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

4. Aligned macOS image alpha/lifetime semantics toward Windows behavior:
- `Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.cpp`
  - `ClampAlpha` floor changed from `0.15` to `0.0`.
- `Platform/macos/Wasm/MacosWasmOverlayRenderMath.cpp`
  - image life clamp changed from `80..6000` to `60..10000`, zero fallback from `300` to `350`.
- `Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Image.cpp`
  - zero `lifeMs` now maps to `max(60, config.icon.durationMs)` before render-plan clamp (matching Windows config-driven fallback intent).

5. Added script-level affine observability gate:
- New test route: `POST /api/wasm/test-resolve-image-affine` (test-gated under existing wasm test env flag).
- Route returns normalized image command diagnostics including:
  - `resolved_x_int`, `resolved_y_int`
  - `resolved_scale_milli`
  - `resolved_rotation_millirad`
- Regression and manual selfcheck now assert three affine cases:
  - translate only (affine disabled)
  - scale matrix (affine enabled)
  - rotation matrix (affine enabled, 90deg)

## Validation
1. Core wasm contract:
```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope wasm --build-dir /tmp/mfx-platform-macos-core-automation-build
```

2. Full wasm regression suite:
```bash
./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto --skip-automation-test
```

3. macOS manual wasm selfcheck:
```bash
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build --build-dir /tmp/mfx-platform-macos-core-automation-build
```

## Result
- `spawn_image_affine` semantics are now aligned by construction across macOS and Windows command execution paths.
- Future affine behavior tuning is centralized in one Core resolver to prevent platform drift.
- macOS image transparency and default lifetime behavior are now materially closer to Windows under the same command/config inputs.
- Affine semantics now have HTTP-contract-level assertions, reducing reliance on manual visual verification.
