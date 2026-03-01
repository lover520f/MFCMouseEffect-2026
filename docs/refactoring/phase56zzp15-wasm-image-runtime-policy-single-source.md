# phase56zzp15: wasm image runtime policy single-source (windows + macOS)

## Context
- Verdict: `Bug/regression risk`.
- `spawn_image` command runtime normalization (scale/alpha/delay/life/applyTint) was duplicated:
  - Windows in `WasmClickCommandExecutor.cpp`
  - macOS in `MacosWasmCommandRenderDispatch.Image.cpp` + renderer support clamps
- This duplication risks silent drift when one side tweaks clamp/fallback values.

## Changes
1. Added shared Core policy helper:
- `MouseFx/Core/Wasm/WasmImageRuntimeConfig.h`
- APIs:
  - `ResolveSpawnImageScale`
  - `ResolveSpawnImageAlpha`
  - `ResolveSpawnImageDelayMs`
  - `ResolveSpawnImageLifeMs`
  - `ResolveSpawnImageApplyTint`

2. Wired both runtime paths to shared policy:
- Windows:
  - `MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`
- macOS:
  - `Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Image.cpp`
  - `Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Plan.cpp` now consumes pre-normalized delay/alpha inputs from request.

3. Removed redundant macOS-local clamp wrappers:
- `Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.h/.cpp`
  - dropped `ClampAlpha` / `ClampDelayMs` API surface.

4. Expanded affine test-route diagnostics:
- `POST /api/wasm/test-resolve-image-affine` now returns normalized runtime fields:
  - `runtime_scale_milli`
  - `runtime_alpha_milli`
  - `runtime_delay_ms`
  - `runtime_life_ms`
  - `runtime_apply_tint`

5. Expanded regression + manual selfcheck assertions:
- Files:
  - `tools/platform/regression/lib/core_http_wasm_contract_dispatch_checks.sh`
  - `tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Added runtime policy checks for:
  - affine translate / scale / rotate scenarios
  - unsigned max clamp (`delay -> 60000`, `life -> 10000`)
  - negative / transparent tint behavior (`runtime_apply_tint=false`)

## Validation
1. Core WASM contract:
```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope wasm --build-dir /tmp/mfx-platform-macos-core-automation-build
```

2. macOS wasm selfcheck:
```bash
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build --build-dir /tmp/mfx-platform-macos-core-automation-build
```

## Result
- `spawn_image` runtime normalization is now single-source across Windows and macOS.
- Contract probes now cover both affine geometry and runtime clamp/fallback semantics, reducing future mac/win drift risk.
