# Phase 28: Serialize Input/Trail Unit Split

## Background
After phase27 split parse input and trail concerns, serialize path still had mixed concerns in one file:
- input-indicator serialization
- trail params/profile serialization

To keep parse/serialize structure symmetric and reduce per-file complexity, serialize path is split the same way.

## Changes

### 1) Split serialize implementation by concern
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.Input.cpp`
  - `BuildInputIndicatorJson(...)`
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.Trail.cpp`
  - `BuildTrailProfilesJson(...)`
  - `BuildTrailParamsJson(...)`
- Removed `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.InputTrail.cpp`

### 2) Project compile item update
- Updated `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - removed old `Serialize.InputTrail.cpp`
  - added `Serialize.Input.cpp` and `Serialize.Trail.cpp`

## Outcome
- Parse and serialize paths now share the same modular shape:
  - common/root
  - input
  - trail
  - effects
- Each unit is smaller and single-purpose.
- Behavior and JSON schema remain unchanged.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
