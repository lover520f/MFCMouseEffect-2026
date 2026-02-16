# Phase 27: Parse Input/Trail Unit Split

## Background
`EffectConfigJsonCodec.Parse.InputTrail.cpp` still combined two separate responsibilities:
- input-indicator parsing
- trail params/profile parsing

To further enforce single responsibility and keep files small, parse units are split by domain concern.

## Changes

### 1) Split parse implementation file by concern
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.Input.cpp`
  - `ParseInputIndicator(...)`
  - local shared helper for input common fields
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.Trail.cpp`
  - `ParseTrailParams(...)`
  - `ParseTrailProfiles(...)`
- Removed `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.InputTrail.cpp`

### 2) Project compile item update
- Updated `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - removed old `Parse.InputTrail.cpp`
  - added `Parse.Input.cpp` and `Parse.Trail.cpp`

## Outcome
- Parse path is now aligned by explicit domain modules:
  - common utils
  - input
  - trail
  - effects
  - root orchestrator
- Each file is shorter and easier to review.
- Behavior and JSON schema remain unchanged.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
