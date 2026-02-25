# Phase 53aa - WebSettings WASM Runtime Endpoint Routes Layer Split

## Background
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeStateRoutes.cpp` mixed toggle and policy responsibilities.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeActionRoutes.cpp` mixed reload and manifest-load responsibilities.
- This increased per-file change surface when extending runtime control-plane APIs.

## Decision
- Keep external runtime route entry contracts unchanged:
  - `HandleWebSettingsWasmRuntimeStateApiRoute(...)`
  - `HandleWebSettingsWasmRuntimeActionApiRoute(...)`
- Split state/action internals by endpoint ownership.
- Keep `WasmRuntimeStateRoutes.cpp` and `WasmRuntimeActionRoutes.cpp` as delegators.

## Code Changes
1. Split runtime state routes by endpoint ownership
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeToggleRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeToggleRoutes.cpp`
- Owns:
  - `POST /api/wasm/enable`
  - `POST /api/wasm/disable`

- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimePolicyRoute.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimePolicyRoute.cpp`
- Owns:
  - `POST /api/wasm/policy`

- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeStateRoutes.cpp`
- Now delegates to toggle route, then policy route.

2. Split runtime action routes by endpoint ownership
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmReloadRoute.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmReloadRoute.cpp`
- Owns:
  - `POST /api/wasm/reload`

- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmLoadManifestRoute.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmLoadManifestRoute.cpp`
- Owns:
  - `POST /api/wasm/load-manifest`

- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmRuntimeActionRoutes.cpp`
- Now delegates to reload route, then load-manifest route.

3. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added new runtime route source entries:
  - `WebSettingsServer.WasmRuntimeToggleRoutes.cpp`
  - `WebSettingsServer.WasmRuntimePolicyRoute.cpp`
  - `WebSettingsServer.WasmReloadRoute.cpp`
  - `WebSettingsServer.WasmLoadManifestRoute.cpp`

## Behavior Compatibility
- Existing route paths, request payloads, and response payloads are unchanged.
- This phase is structure-only and preserves runtime behavior contracts.

## Functional Ownership
- Category: `WASM`
- Coverage: WebSettings WASM runtime control-plane routes (`enable/disable/policy/reload/load-manifest`).

## Verification
1. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.

2. `./tools/docs/doc-hygiene-check.sh --strict`
- Result: passed.
