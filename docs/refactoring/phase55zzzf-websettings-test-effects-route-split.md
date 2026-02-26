# phase55zzzf: WebSettings test effects routes split

## Scope
- Capability bucket: `effects` (test probe API maintainability).
- Goal: split monolithic `WebSettingsServer.TestEffectsApiRoutes.cpp` into single-responsibility route units without changing API behavior.

## Change Summary
1. Kept `HandleWebSettingsTestEffectsApiRoute` as top-level delegator.
2. Added profile route module:
   - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsProfileApiRoute.h`
   - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsProfileApiRoute.cpp`
3. Added overlay route module:
   - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsOverlayApiRoute.h`
   - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsOverlayApiRoute.cpp`
4. Updated POSIX CMake wiring:
   - `MFCMouseEffect/Platform/CMakeLists.txt` includes both new route translation units.

## Contract Invariants
- `GET /api/effects/test-render-profiles`: unchanged payload/guard semantics.
- `POST /api/effects/test-overlay-windows`: unchanged payload/guard semantics and arithmetic/lifecycle fields.
- Existing env gate `MFX_ENABLE_EFFECT_OVERLAY_TEST_API` unchanged.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`
