# Phase 53p - WebSettings WASM Routes Runtime/Catalog/Utils Split

## Background
- `WebSettingsServer.WasmRoutes.cpp` had grown to a single large implementation that mixed:
  - runtime control APIs
  - catalog/import/export APIs
  - helper/parsing/response utilities
- This increased change risk for WASM route evolution.

## Decision
- Keep existing WASM API contracts unchanged.
- Split WASM route implementation into three focused layers:
  - runtime routes
  - catalog/transfer routes
  - shared wasm-route utilities
- Keep `WebSettingsServer.WasmRoutes.cpp` as delegation entry only.

## Code Changes
1. Shared utility layer
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRouteUtils.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRouteUtils.cpp`
- Centralized:
  - JSON response helper
  - payload parsing helpers
  - manifest path compare helper
  - WASM diagnostics/state response builders

2. Catalog/transfer route layer
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmCatalogRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmCatalogRoutes.cpp`
- Handles:
  - `/api/wasm/catalog`
  - `/api/wasm/import-selected`
  - `/api/wasm/import-from-folder-dialog`
  - `/api/wasm/export-all`

3. Runtime route layer
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeRoutes.cpp`
- Handles:
  - `/api/wasm/enable`
  - `/api/wasm/disable`
  - `/api/wasm/policy`
  - `/api/wasm/reload`
  - `/api/wasm/load-manifest`

4. WASM entry route file simplification
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRoutes.cpp`
- Now delegates to runtime and catalog route modules only.

5. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added all new wasm route split source files.

## Behavior Compatibility
- Endpoint paths, payload schema, and diagnostics fields remain unchanged.
- No behavioral change to wasm enable/load/reload/policy or plugin catalog/import/export flow.
- This phase is structure-only.

## Functional Ownership
- Category: `WASM`
- Coverage: plugin runtime control + plugin catalog/transfer WebSettings API path

## Verification
1. `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- Result: passed.

2. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.
