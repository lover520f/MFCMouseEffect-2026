# Phase 53w - WebSettings WASM Catalog Routes Layer Split

## Background
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmCatalogRoutes.cpp` previously mixed:
  - plugin catalog query route
  - plugin import routes (`import-selected`, `import-from-folder-dialog`)
  - plugin export route
- This increased coupling and made WASM route changes higher risk.

## Decision
- Keep WASM API paths and response contracts unchanged.
- Split by responsibility:
  - catalog query layer
  - import layer
  - export layer
- Keep `WebSettingsServer.WasmCatalogRoutes.cpp` as thin delegating entry.

## Code Changes
1. Added catalog query route layer
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmCatalogQueryRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmCatalogQueryRoutes.cpp`
- Owns:
  - `POST /api/wasm/catalog`

2. Added import route layer
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmImportRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmImportRoutes.cpp`
- Owns:
  - `POST /api/wasm/import-selected`
  - `POST /api/wasm/import-from-folder-dialog`
- Preserves probe-only and folder-dialog behaviors.

3. Added export route layer
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmExportRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmExportRoutes.cpp`
- Owns:
  - `POST /api/wasm/export-all`

4. Delegator and build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmCatalogRoutes.cpp`
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Behavior Compatibility
- Endpoint paths and payload fields unchanged.
- Import/export semantics unchanged.
- Native folder picker import dialog and probe-only contract unchanged.
- This phase is structure-only refactor.

## Functional Ownership
- Category: `WASM`
- Coverage: plugin catalog/import/export HTTP control-plane routes.

## Verification
1. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.
