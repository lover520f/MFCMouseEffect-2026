# Phase 56zzj - macOS Trail "none" Hard Disable and Contract Gate

## Scope
- Capability: `effects` (trail category).
- Goal:
  - make `trail=none` a true disable state across shared compute + mac renderer + Swift bridge.
  - add regression contract coverage so `none` cannot silently regress to `line`.

## Decision
- `none` must be terminal in the trail command path:
  - compute layer: `emit=false`.
  - renderer layer: explicit short-circuit.
  - Swift bridge: do not create overlay for `none`.
- Keep style-layer normalization consistent (`none -> none`) for future call-sites.

## Code Changes
1. Shared compute hard-disable:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Effects/TrailEffectCompute.cpp`
  - `ComputeTrailEffectRenderCommand(...)` now returns early with `emit=false` when normalized type is `none`.

2. mac renderer + Swift bridge guards:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.cpp`
  - skip render when `normalizedType == "none"`.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayBridge.swift`
  - `mfxNormalizeTrailType("none")` keeps `none`.
  - overlay create path returns `nil` for `none`.
  - path builder returns empty path for `none`.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayStyle.cpp`
  - normalization now keeps `none`.
  - line-path builder returns empty path for `none`.

3. Contract gate:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
  - add alias assertion: `trail input=none -> normalized=none`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`

All commands passed on macOS.

## Result
- `trail=none` no longer falls through to visible line rendering.
- Trail disable semantics are now enforced by contract tests.
