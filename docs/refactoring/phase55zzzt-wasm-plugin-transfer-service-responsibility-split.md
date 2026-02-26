# Phase 55zzzt - WASM plugin transfer service responsibility split

## Summary
- Capability: `wasm` (plugin import/export transfer path).
- Goal: split `WasmPluginTransferService` by responsibility while keeping API/error-code behavior unchanged.

## Changes
1. Introduced internal contract boundary
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.Internal.h`
- Purpose:
  - shared helper contracts;
  - import/export implementation entry contracts.

2. Split transfer logic into focused translation units
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.Common.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.Import.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.Export.cpp`
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp` (delegation-only).

3. Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added split units to runtime target source list.

## Why
- Original file mixed shared path utilities, import validation/copy flow, and export discovery/copy flow.
- Split reduces cross-domain coupling and lowers future WASM transfer change risk.

## Validation
```bash
./tools/platform/regression/run-posix-regression-suite.sh --platform auto
./tools/docs/doc-hygiene-check.sh --strict
```

## Compatibility
- No API or schema changes.
- Existing import/export error codes and field semantics preserved.
