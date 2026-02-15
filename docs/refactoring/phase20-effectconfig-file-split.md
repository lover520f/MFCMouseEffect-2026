# Phase 20: EffectConfig File Split

## Background
`MouseFx/Core/EffectConfig.cpp` had grown to 500+ lines and mixed four responsibilities:
- shared config sanitization/encoding helpers
- load-time JSON parsing
- save-time JSON serialization
- public utility methods (`ArgbFromHex`, default config, trail profile lookup)

This made it harder to review, test, and safely evolve config behavior.

## Changes

### 1) Split by responsibility
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigInternal.h`
- Added `MFCMouseEffect/MouseFx/Core/EffectConfig.Internal.cpp`
- Added `MFCMouseEffect/MouseFx/Core/EffectConfig.Load.cpp`
- Added `MFCMouseEffect/MouseFx/Core/EffectConfig.Save.cpp`
- Reduced `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp` to public entry points only

### 2) Introduced internal helper boundary
Internal shared helpers are now centralized under `mousefx::config_internal`:
- encoding and IO helpers (`ReadFileAsUtf8`, `WStringToUtf8`, `ArgbToHex`)
- value normalization/sanitization (`NormalizeHoldFollowMode`, trail/input sanitizers)

`Load` and `Save` now depend on these shared helpers instead of repeating logic.

### 3) Structured load/save flow
- `EffectConfig.Load.cpp` groups parsing into focused functions:
  - input indicator
  - trail params
  - trail profiles
  - per-effect payload parsing
- `EffectConfig.Save.cpp` groups serialization into focused builders:
  - input indicator payload
  - trail profile payload
  - trail params payload
  - active effects payload
  - effects payload

## Safety and Compatibility
- JSON schema and key names are kept unchanged.
- Legacy `mouse_indicator` fallback parsing remains intact.
- Existing behavior for default file recreation on parse failure remains intact.

Additionally, UTF conversion buffer sizing was hardened in the shared helpers to avoid writing with `len` into `len-1` sized buffers.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
