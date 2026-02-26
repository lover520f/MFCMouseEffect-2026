# Phase 55zzzd: macOS Effect Type Probe And Hold Style Mapping

## Capability
- Effects

## Why
- Overlay probe route validated category counts, but mostly exercised default types.
- macOS hold rendering needed explicit style mapping for multiple hold type families (including GPU-route aliases) to keep visible behavior differentiated.

## Scope
- Add explicit hold type -> style mapping for macOS hold overlay visuals.
- Extend effect probe route to accept per-category type arguments and report them in response payload.
- Exercise non-default type matrix in core automation contract checks.

## Code Changes
1. macOS hold style mapping and visuals:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRenderer.mm`
2. macOS hover alias normalization (`suspension` -> `tubes`):
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRenderer.mm`
3. effect probe API type arguments and response echoes:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsApiRoutes.cpp`
4. automation contract probe payload now exercises non-default type matrix:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`

## Validation
- `cmake --build /tmp/mfx-platform-macos-core-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- `/api/effects/test-overlay-windows` supports `click_type/trail_type/scroll_type/hold_type/hover_type` inputs and mirrors them in response.
- Hold styles on macOS now explicitly differentiate charge/lightning/hex/tech/neon/hologram/quantum_halo/flux_field families.
