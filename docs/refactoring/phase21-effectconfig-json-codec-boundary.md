# Phase 21: EffectConfig JSON Codec Boundary

## Background
After phase20, `EffectConfig` had been split into multiple files, but `EffectConfig.Load.cpp` and `EffectConfig.Save.cpp` still contained most JSON field mapping details.
This kept entry methods longer than necessary and mixed two concerns:
- lifecycle/file handling (`Load`/`Save`)
- schema mapping (json <-> config model)

## Changes

### 1) Introduced a dedicated codec boundary
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.h`
  - `config_json::ApplyRootToConfig(...)`
  - `config_json::BuildRootFromConfig(...)`

### 2) Split JSON mapping implementation by direction
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.cpp`
  - parses root-level fields, active effect selections, input indicator, trail params/profiles, and per-effect payloads
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.cpp`
  - builds input/trail/active/effects JSON sections and final root payload

### 3) Simplified `Load`/`Save` entry methods
- `MFCMouseEffect/MouseFx/Core/EffectConfig.Load.cpp`
  - now focuses on file read + parse error handling + calling codec parser
- `MFCMouseEffect/MouseFx/Core/EffectConfig.Save.cpp`
  - now focuses on codec build + file write

## Why this improves architecture
- Separates IO/lifecycle concerns from schema mapping concerns.
- Makes each file smaller and easier to reason about.
- Reduces regression risk when extending config schema: most edits are isolated to codec files.

## Compatibility
- No config schema key changed.
- Legacy `mouse_indicator` fallback remains supported in parser path.
- Existing behavior for invalid/corrupted config recreation is unchanged.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
