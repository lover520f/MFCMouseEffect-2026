# phase56zzp12: macos wasm text overlay shared fallback + kinematics parity

## Context
- Verdict: `Bug/regression`.
- Symptom: macOS WASM `spawn_text` used a standalone dark panel renderer (background + border), visually diverging from Windows floating-text behavior and duplicating text animation logic.
- Risk: duplicated text-command semantics caused drift in `lifeMs/scale/floatDistance` handling between Windows and macOS.

## Changes
1. Unified macOS WASM text rendering path with shared fallback:
- `Platform/macos/Wasm/MacosWasmTextOverlay.cpp` now routes `spawn_text` to `MacosTextEffectFallback` (shared floating text path) instead of the previous dedicated WASM text panel bridge.
- Overlay slot throttle semantics are preserved by keeping the existing admission/release flow (`TryAcquireWasmOverlaySlot` + delayed `ReleaseWasmOverlaySlot`).

2. Aligned text-command parameter mapping with Windows semantics:
- `Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Text.cpp` now computes `TextConfig` using the same kinematics rules as Windows (`lifeMs` clamp, `scale` to font size, `vy/ay` to float distance).
- The dispatch layer passes resolved `TextConfig` into `ShowWasmTextOverlay(...)` so renderer no longer re-infers timing/size from partial fields.

3. Removed obsolete macOS WASM text bridge residuals:
- Deleted:
  - `Platform/macos/Wasm/MacosWasmTextOverlayBridge.swift`
  - `Platform/macos/Wasm/MacosWasmTextOverlaySwiftBridge.h`
  - `Platform/macos/Wasm/MacosWasmTextOverlay.Internal.h`
  - `Platform/macos/Wasm/MacosWasmTextOverlay.Layout.cpp`
- Added shared fallback accessor:
  - `Platform/macos/Wasm/MacosWasmTextOverlayFallback.h`
  - `Platform/macos/Wasm/MacosWasmTextOverlayFallback.cpp`
- Updated `Platform/macos/CMakeLists.txt` to remove old text bridge compilation and wire new shared fallback unit.

4. Promoted text-command mapping to Core single-source helper:
- Added `MouseFx/Core/Wasm/WasmTextCommandConfig.h` (`BuildSpawnTextConfig(...)`).
- Windows `WasmClickCommandExecutor` and macOS `MacosWasmCommandRenderDispatch.Text.cpp` now call the same helper for `lifeMs/scale/vy/ay -> TextConfig` mapping.

## Validation
1. Build:
```bash
cmake -S MFCMouseEffect/Platform -B /tmp/mfx-platform-macos-core-automation-build -DMFX_PACKAGE_PLATFORM=macos -DMFX_ENABLE_POSIX_CORE_RUNTIME=ON -DMFX_ENABLE_ENTRY_RUNTIME_TARGETS=ON
cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8
```

2. WASM contract:
```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope wasm --build-dir /tmp/mfx-platform-macos-core-automation-build
```

3. Suite gate:
```bash
./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto --skip-automation-test
```

## Result
- macOS WASM text now follows shared floating-text visual semantics and Windows-equivalent command mapping behavior.
- Old standalone WASM text-overlay bridge path is removed to reduce duplication and drift surface.
