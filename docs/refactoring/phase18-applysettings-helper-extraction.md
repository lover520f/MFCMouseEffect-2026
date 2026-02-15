# Phase 18: Apply-Settings Helper Extraction

## Summary

Refactored `CommandHandler.ApplySettings.cpp` by extracting major payload-processing blocks into focused helper functions:

- payload parse/validation
- active effect apply
- text payload apply
- input-indicator payload apply
- trail tuning payload apply

Goal: reduce `HandleApplySettings(...)` cognitive load while preserving processing order and runtime behavior.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/CommandHandler.ApplySettings.cpp` | Added helper functions in anonymous namespace and simplified `HandleApplySettings(...)` orchestration flow |

### Extracted Helpers

- `TryParsePayloadObject(...)`
- `ApplyActiveSettings(...)`
- `ParseCsvUtf8TextList(...)`
- `ApplyTextSettings(...)`
- `ApplyInputIndicatorFields(...)`
- `ApplyInputIndicatorSettings(...)`
- `ApplyTrailTuningSettings(...)`

## Behavior Compatibility

- No intentional behavior changes.
- Processing order in `HandleApplySettings(...)` remains:
  1. active
  2. ui language
  3. text
  4. input indicator
  5. hold follow mode
  6. trail tuning
  7. theme (last)

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
