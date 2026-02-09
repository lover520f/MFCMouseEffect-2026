# Dawn Backend + CPU Fallback (Stage 30 Dawn Build Flag Activation)

## Goal
- Enable actual Dawn runtime initialization path at compile time.
- Keep existing CPU fallback behavior unchanged.

## Changes

### 1) Project Build Macro Update
- Updated:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
- Added `MOUSEFX_ENABLE_DAWN` to all four `ClCompile` configuration groups:
  - `Debug|x64`
  - `Debug|Win32`
  - `Release|Win32`
  - `Release|x64`
- Existing `MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE` remains enabled.

## Why
- Before this stage, overlay bridge diagnostics existed, but core Dawn init flow in `DawnRuntime.cpp` was gated off by `#ifdef MOUSEFX_ENABLE_DAWN`.
- With this flag enabled, runtime now attempts loader/symbol/instance/adapter/device handshake and reports detailed readiness/fallback states.

## Compatibility
- CPU fallback remains the default safety path when Dawn runtime or GPU capability is unavailable.
- No behavior regression for machines without Dawn DLLs: status will report loader/symbol failures and continue on CPU.
