# phase56zzp21: wasm image counter relation gates

## Context
- Verdict: `Regression-gap risk`.
- macOS image-overlay diagnostics counters were added, but initial checks only covered monotonic increment and one upper-bound relation.
- Missing guard: no full set-relation validation across `total/with_asset/tint/tint_with_asset`.

## Changes
1. Strengthened dispatch diagnostics assertions:
- Regression helper:
  - `tools/platform/regression/lib/core_http_wasm_dispatch_assert_helpers.sh`
- Manual helper:
  - `tools/platform/manual/lib/wasm_selfcheck_dispatch_assert_helpers.sh`
- Added enforced relations (macOS):
  - `requests_with_asset <= requests`
  - `apply_tint_requests <= requests`
  - `apply_tint_requests_with_asset <= apply_tint_requests`
  - `apply_tint_requests_with_asset <= requests_with_asset`

2. Expanded state/schema key exposure checks:
- `tools/platform/regression/lib/core_http_state_checks.sh`
- Added state/schema assertions for:
  - `mac_image_overlay_requests`
  - `mac_image_overlay_requests_with_asset`
  - `mac_image_overlay_apply_tint_requests`
  - `mac_image_overlay_apply_tint_requests_with_asset`

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
- macOS image overlay diagnostics are now guarded by stronger mathematical invariants instead of only monotonic checks.
- Counter regressions caused by partial path breakage (asset detection/tint tagging) are easier to catch automatically.
