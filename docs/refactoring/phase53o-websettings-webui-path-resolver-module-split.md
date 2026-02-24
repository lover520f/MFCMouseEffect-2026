# Phase 53o - WebSettings WebUI Path Resolver Module Split

## Background
- `WebSettingsServer.cpp` still embedded all WebUI base-directory discovery logic.
- This made server bootstrap and path-resolution concerns tightly coupled in one file.

## Decision
- Keep WebUI discovery behavior unchanged.
- Extract directory discovery logic into a dedicated resolver module:
  - `WebSettingsServer.WebUiPathResolver.h`
  - `WebSettingsServer.WebUiPathResolver.cpp`

## Code Changes
1. New resolver module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WebUiPathResolver.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WebUiPathResolver.cpp`
- Exposed function:
  - `ResolveWebSettingsWebUiBaseDir()`

2. Server bootstrap simplification
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Constructor now delegates WebUI base path resolution to the resolver module.

3. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added `WebSettingsServer.WebUiPathResolver.cpp` to runtime source list.

## Behavior Compatibility
- Discovery precedence is preserved:
  - `MFX_WEBUI_DIR`
  - executable `webui` directory
  - working-directory candidate set
  - source-tree fallback
- This phase is structure-only and does not change user-visible behavior.

## Functional Ownership
- Category: `共用控制面`
- Coverage: settings entry used by `特效 / WASM / 键鼠指示 / 手势映射`

## Verification
1. `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- Result: passed.

2. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.
