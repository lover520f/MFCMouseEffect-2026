# Phase 26: Effect Color Mapping Helper Extraction

## Background
After parse/serialize modularization, effect color mapping logic still had repeated blocks:
- ripple button colors (`fill/stroke/glow`) in parse and serialize paths
- icon `fill/stroke` handling in parse and serialize paths

This repetition increased maintenance cost and made schema-level color changes harder to keep consistent.

## Changes

### 1) Added shared effect-color helper module
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodecEffectsColorHelpers.h`
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodecEffectsColorHelpers.cpp`

Provides focused helpers:
- `ParseRippleButtonColors(...)`
- `ParseFillStrokeColors(...)`
- `BuildRippleButtonColors(...)`
- `BuildFillStrokeColors(...)`

### 2) Reused helpers in parse/serialize effect codec
- Updated `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.Effects.cpp`
  - replaced repeated left/right/middle color extraction blocks
  - replaced icon fill/stroke parse block
- Updated `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.Effects.cpp`
  - replaced repeated ripple button color serialization blocks
  - replaced icon fill/stroke serialization block

### 3) Project registration
- Updated `MFCMouseEffect/MFCMouseEffect.vcxproj` for new helper header/source.

## Outcome
- Eliminates duplicated color mapping branches across parse and serialize.
- Keeps color-field handling behavior in a single place.
- Reduces future regression risk when color schema evolves.

## Compatibility
- JSON schema keys and value formats are unchanged.
- Runtime behavior remains the same.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
