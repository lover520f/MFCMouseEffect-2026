# Phase 53k - WebSettings WASM Routes Module Split

## Background
- `WebSettingsServer.Routing.cpp` had already split test-only endpoints into `WebSettingsServer.TestApiRoutes.*`, but all production `/api/wasm/*` routes were still embedded in the main routing file.
- This created another high-coupling hotspot and made further WASM route changes higher risk.

## Decision
- Keep route behavior and contracts unchanged.
- Split `/api/wasm/*` handling into a dedicated production module:
  - `WebSettingsServer.WasmRoutes.h`
  - `WebSettingsServer.WasmRoutes.cpp`
- Main routing file keeps only delegation:
  - `HandleWebSettingsWasmApiRoute(req, path, controller_, resp)`

## Code Changes
1. New module for WASM route handling
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRoutes.cpp`
- Kept all existing WASM APIs in one place:
  - `POST /api/wasm/enable`
  - `POST /api/wasm/disable`
  - `POST /api/wasm/reload`
  - `POST /api/wasm/load-manifest`
  - `POST /api/wasm/policy`
  - `POST /api/wasm/catalog`
  - `POST /api/wasm/import-selected`
  - `POST /api/wasm/import-from-folder-dialog`
  - `POST /api/wasm/export-all`

2. Main route file reduced
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- Removed inline WASM route implementation and route-specific helper functions.
- Added include and delegation call to wasm routes module.

3. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added `WebSettingsServer.WasmRoutes.cpp` to runtime common source list.

## Behavior Compatibility
- API paths, payloads, and response structures remain unchanged.
- Existing permission/picker/WASM runtime behavior remains unchanged.
- This phase is a structural refactor only.

## Verification
1. `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- Result: passed.

2. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed (scaffold/core smoke/core automation/macOS injection selfcheck/macOS wasm selfcheck/linux compile gate/webui automation semantics).

## Risks
- Risk: accidental route mismatch after move.
- Mitigation: route-path behavior covered by existing core automation contracts and full POSIX suite.
