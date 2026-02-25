# Phase 53y - WebSettings WASM Route Utils Split

## Background
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRouteUtils.cpp` still combined three responsibilities:
  - request payload parsing
  - manifest path comparison
  - WASM response body construction
- This coupling made utility-level changes harder to review and extend.

## Decision
- Keep `WebSettingsServer.WasmRouteUtils.h` public API unchanged.
- Split implementations by responsibility:
  - parse utils
  - path utils
  - response utils
- Remove monolithic `WasmRouteUtils.cpp` implementation file.

## Code Changes
1. Added parse utils implementation
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRouteParseUtils.cpp`
- Owns:
  - `ParseObjectOrEmpty`
  - `ParseManifestPathUtf8`
  - `ParseInitialPathUtf8`

2. Added path utils implementation
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRoutePathUtils.cpp`
- Owns:
  - `IsSameManifestPath`

3. Added response utils implementation
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRouteResponseUtils.cpp`
- Owns:
  - `SetJsonResponse`
  - `BuildWasmResponse`
  - `BuildWasmActionResponse`

4. Removed monolithic implementation
- Removed `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRouteUtils.cpp`

5. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Replaced old source entry with:
  - `WebSettingsServer.WasmRouteParseUtils.cpp`
  - `WebSettingsServer.WasmRoutePathUtils.cpp`
  - `WebSettingsServer.WasmRouteResponseUtils.cpp`

## Behavior Compatibility
- Public utility function names and signatures unchanged.
- All WASM route call sites keep existing behavior.
- This phase is structure-only refactor.

## Functional Ownership
- Category: `WASM`
- Coverage: shared WASM WebSettings route utility layer used by runtime/catalog/import/export routes.

## Verification
1. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.
