# Phase 23: EffectConfig Serialize Multi-Unit Split

## Background
After phase22, parse flow was already split into focused units, but serialize flow was still centralized in one file (`EffectConfigJsonCodec.Serialize.cpp`).
This kept write-path responsibilities coupled and made schema-extension edits less localized.

## Changes

### 1) Added serialize-internal boundary
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodecSerializeInternal.h`
  - declares serialize sections:
    - `BuildInputIndicatorJson`
    - `BuildTrailProfilesJson`
    - `BuildTrailParamsJson`
    - `BuildActiveEffectsJson`
    - `BuildEffectsJson`

### 2) Split serialize implementation by concern
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.InputTrail.cpp`
  - input-indicator payload
  - trail profiles payload
  - trail renderer params payload
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.Effects.cpp`
  - active effect selection payload
  - effect payloads (ripple/trail/icon/text)
- Updated `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.cpp`
  - now only orchestrates root JSON assembly

### 3) Project registration
- Updated `MFCMouseEffect/MFCMouseEffect.vcxproj` to include new header/source files.

## Outcome
- Serialize path now matches parse path in modularity.
- `Serialize.cpp` remains small and focused on high-level root composition.
- Schema keys and runtime behavior are unchanged.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
