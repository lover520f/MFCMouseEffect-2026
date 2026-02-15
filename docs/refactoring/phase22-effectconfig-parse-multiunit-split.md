# Phase 22: EffectConfig Parse Multi-Unit Split

## Background
`EffectConfigJsonCodec.Parse.cpp` still mixed multiple parsing concerns in one file:
- root/active fields
- input indicator and trail payloads
- effects payloads
- UTF conversion helper

This made parser evolution harder and increased review complexity.

## Changes

### 1) Added parser-internal boundary
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodecParseInternal.h`
  - shared parser helper templates (`GetOr`, `GetColorOr`)
  - declarations for parser sections (`ParseInputIndicator`, `ParseTrailParams`, `ParseTrailProfiles`, `ParseEffects`)
  - declaration of UTF conversion helper

### 2) Split parse implementation by concern
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.Common.cpp`
  - `TryUtf8ToWide`
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.InputTrail.cpp`
  - input indicator + legacy mouse indicator parsing
  - trail params and trail profile parsing
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.Effects.cpp`
  - ripple/trail/icon/text effect parsing
- Updated `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.cpp`
  - now only handles root/active fields and orchestrates section parsers

## Outcome
- Parse logic is now modular and closer to single-responsibility.
- File sizes are significantly smaller and easier to review incrementally.
- Behavior and schema keys remain unchanged.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
