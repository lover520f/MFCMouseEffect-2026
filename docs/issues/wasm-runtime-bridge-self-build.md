# WASM Runtime DLL Self-Build Landing

## Context

The WASM host already expected a dynamic bridge DLL:
- `mfx_wasm_runtime.dll`

But the repo did not include a buildable project for that DLL, so runtime always fell back to `null` backend unless users provided an external binary manually.

## Goal

Make `mfx_wasm_runtime.dll` first-party and reproducible from this repository:
- build with the same Visual Studio/MSBuild flow as the app,
- output beside `MFCMouseEffect.exe`,
- package it in installer.

## Implementation

### 1. New runtime bridge project

Added project:
- `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj`

Added bridge sources:
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeExports.cpp`
- `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.def`

Added third-party runtime source (vendored):
- `MFCMouseEffect/WasmRuntimeBridge/third_party/wasm3/*`

### 2. ABI-compatible exports

The DLL now exports all host-required symbols:
- `mfx_wasm_runtime_create`
- `mfx_wasm_runtime_destroy`
- `mfx_wasm_runtime_load_module_file`
- `mfx_wasm_runtime_unload_module`
- `mfx_wasm_runtime_is_module_loaded`
- `mfx_wasm_runtime_call_get_api_version`
- `mfx_wasm_runtime_call_on_event`
- `mfx_wasm_runtime_reset_plugin`
- `mfx_wasm_runtime_last_error`

Export names are fixed via `.def` to keep x86/x64 lookup consistent.

### 3. Build graph wiring

Updated:
- `MFCMouseEffect.slnx` (include runtime bridge project)
- `MFCMouseEffect/MFCMouseEffect.vcxproj` (project reference to runtime bridge)

Result:
- Building `MFCMouseEffect` now also builds runtime bridge.
- `mfx_wasm_runtime.dll` lands in `x64/Debug` or `x64/Release` beside the exe.

### 4. Installer wiring

Updated:
- `Install/MFCMouseEffect.iss`

Added:
- preflight check for `x64/Release/mfx_wasm_runtime.dll`
- packaging rule to copy this DLL into `{app}`

## Runtime behavior

With this landing:
- no external download is required for runtime DLL,
- fallback reason `Cannot load mfx_wasm_runtime.dll` should disappear once solution build succeeds.

If runtime still falls back:
- check `wasm.runtime_fallback_reason` and `wasm.last_error`,
- verify `mfx_wasm_runtime.dll` exists beside `MFCMouseEffect.exe`.
