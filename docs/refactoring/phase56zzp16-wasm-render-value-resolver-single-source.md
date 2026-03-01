# phase56zzp16: wasm render value resolver single-source

## Context
- Verdict: `Bug/regression risk`.
- Text/color/tint fallback resolution logic existed in multiple places:
  - Core `WasmRenderResourceResolver`
  - macOS `MacosWasmCommandRenderResolvers`
  - test-route local helper logic
- Multi-point duplication increased drift risk when changing fallback semantics.

## Changes
1. Added shared resolver:
- `MouseFx/Core/Wasm/WasmRenderValueResolver.h`
- Provides pure value-level helpers:
  - `HasVisibleAlpha`
  - `ResolveTextById`
  - `ResolveTextColorArgb`
  - `ResolveImageTintArgb`

2. Rewired call sites to shared helper:
- Core:
  - `MouseFx/Core/Wasm/WasmRenderResourceResolver.cpp`
- macOS:
  - `Platform/macos/Wasm/MacosWasmCommandRenderResolvers.cpp`
- test route:
  - `MouseFx/Server/WebSettingsServer.TestWasmInputApiRoutes.cpp`

3. Resulting behavior scope:
- Fallback text pool (`WASM/MouseFx/Click`) now has a single implementation source.
- Command color alpha gate and icon tint fallback now share one implementation across runtime and contract route diagnostics.

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
- Text/color/tint value resolution is now single-source for Windows/macOS runtime paths and test contracts.
- Future fallback or color-policy changes only need one code-point update.
