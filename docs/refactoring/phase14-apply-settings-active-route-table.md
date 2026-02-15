# Phase 14: Apply-Settings Active Route Table

## Summary

Replaced the hard-coded five-call active-effect application sequence in `HandleApplySettings(...)` with a small route table loop.

Goal: keep active-setting key/category mapping centralized and reduce repetitive call sites.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/CommandHandler.cpp` | Added `kActiveSettingRoutes` in `HandleApplySettings(...)` and iterated it instead of manual calls |

### Mapping Covered

- `click` -> `EffectCategory::Click`
- `trail` -> `EffectCategory::Trail`
- `scroll` -> `EffectCategory::Scroll`
- `hold` -> `EffectCategory::Hold`
- `hover` -> `EffectCategory::Hover`

## Behavior Compatibility

- No intentional behavior changes.
- Active-setting apply order remains the same as the previous manual sequence.

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
