# Phase 24: EffectConfig JSON Keys Centralization

## Background
After parse/serialize multi-unit split, `EffectConfigJsonCodec` still repeated many JSON key literals across multiple files.
This increased maintenance cost and made schema changes error-prone.

## Changes

### 1) Added centralized key definitions
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonKeys.h`
- Defines stable constants for all config schema keys:
  - root keys (`default_effect`, `theme`, `effects`, ...)
  - active effect keys
  - input indicator keys
  - trail profile and trail params keys
  - effect payload keys (ripple/trail/icon/text and subfields)

### 2) Replaced codec hardcoded key strings
- Updated parse path:
  - `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.cpp`
  - `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.InputTrail.cpp`
  - `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Parse.Effects.cpp`
- Updated serialize path:
  - `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.cpp`
  - `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.InputTrail.cpp`
  - `MFCMouseEffect/MouseFx/Core/EffectConfigJsonCodec.Serialize.Effects.cpp`

### 3) Project registration
- Updated `MFCMouseEffect/MFCMouseEffect.vcxproj` to include the new key header.

## Outcome
- Removes duplicated string literals in codec implementation files.
- Schema changes now require editing one source of truth (`EffectConfigJsonKeys.h`).
- Reduces typo risk and improves long-term maintainability.

## Compatibility
- No JSON schema behavior change.
- Key names remain identical; only the reference style changed.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
