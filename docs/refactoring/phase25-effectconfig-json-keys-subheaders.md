# Phase 25: EffectConfig JSON Keys Subheaders

## Background
Phase24 centralized config JSON schema keys into a single header, but that file still carried all key groups in one place.
To keep file responsibilities narrow and line counts small, key definitions are further split by concern.

## Changes

### 1) Split key definitions into focused headers
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonKeys.Root.h`
  - root-level schema keys (`default_effect`, `effects`, ...)
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonKeys.Active.h`
  - active effect slot keys
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonKeys.Input.h`
  - input-indicator keys
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonKeys.Trail.h`
  - trail profile and trail-params keys
- Added `MFCMouseEffect/MouseFx/Core/EffectConfigJsonKeys.Effects.h`
  - effect payload keys (ripple/trail/icon/text and click subkeys)

### 2) Keep public include path stable
- Updated `MFCMouseEffect/MouseFx/Core/EffectConfigJsonKeys.h`
  - now acts as a lightweight aggregator that only includes the subheaders
- Existing codec files still include `EffectConfigJsonKeys.h`, so callers remain unchanged.

### 3) Project registration
- Updated `MFCMouseEffect/MFCMouseEffect.vcxproj` to include all new subheaders.

## Outcome
- Each key-header now has a single concern and smaller size.
- Schema maintenance stays centralized but no longer crowded in one file.
- No runtime behavior or JSON schema key changes.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
