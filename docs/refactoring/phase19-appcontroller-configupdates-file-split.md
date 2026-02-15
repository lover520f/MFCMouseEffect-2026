# Phase 19: AppController Config-Updates File Split

## Summary

Split configuration-mutation methods out of `AppController.cpp` into a dedicated implementation file:

- `AppController.cpp`: lifecycle, routing, runtime orchestration focus
- `AppController.ConfigUpdates.cpp`: config update and apply entry points

Goal: improve file-level single responsibility and reduce hotspot file size.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/AppController.cpp` | Removed config update method definitions moved to dedicated file |
| `MFCMouseEffect.vcxproj` | Added `MouseFx/Core/AppController.ConfigUpdates.cpp` compile unit |

### New Files

| File | Responsibility |
|------|----------------|
| `MouseFx/Core/AppController.ConfigUpdates.cpp` | `SetUiLanguage`, text settings updates, input-indicator config apply, trail tuning apply, reset/reload config flows |

## Size Impact

- `AppController.cpp`: reduced to ~536 lines
- `AppController.ConfigUpdates.cpp`: ~73 lines

## Behavior Compatibility

- No intentional behavior changes.
- Moved methods preserve existing call order and side effects.

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
