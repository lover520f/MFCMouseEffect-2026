# Phase 15: Input-Indicator Payload Dedup

## Summary

Deduplicated repeated `input_indicator` / `mouse_indicator` payload parsing in `HandleApplySettings(...)` by introducing one shared field-application lambda.

Goal: remove duplicated field assignment logic while preserving compatibility between new and legacy payload schemas.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/CommandHandler.cpp` | Added `applyInputIndicatorFields(...)` lambda and reused it for both `input_indicator` and legacy `mouse_indicator` branches |

### Compatibility Handling

- `input_indicator` branch:
  - applies common fields
  - applies advanced fields (`target_monitor`, `key_display_mode`, `per_monitor_overrides`)
- `mouse_indicator` branch:
  - applies only common fields (legacy-compatible)
  - advanced fields intentionally not parsed

## Behavior Compatibility

- No intentional behavior changes.
- Legacy payload remains supported with previous field scope.

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
