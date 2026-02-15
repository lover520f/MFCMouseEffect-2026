# Phase 17: CommandHandler Apply-Settings File Split

## Summary

Split `CommandHandler` implementation by responsibility:

- command routing + lightweight command handlers remain in `CommandHandler.cpp`
- heavy `apply_settings` payload parsing moved to `CommandHandler.ApplySettings.cpp`

Goal: improve single-responsibility and reduce maintenance pressure on one large translation unit.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/CommandHandler.cpp` | Removed `HandleApplySettings(...)` implementation; retained command routing path |
| `MFCMouseEffect.vcxproj` | Added new compile unit `MouseFx/Core/CommandHandler.ApplySettings.cpp` |

### New Files

| File | Responsibility |
|------|----------------|
| `MouseFx/Core/CommandHandler.ApplySettings.cpp` | `apply_settings` JSON payload parsing and field-to-config apply logic |

## Size Impact

- `CommandHandler.cpp`: reduced to ~86 lines
- `CommandHandler.ApplySettings.cpp`: ~218 lines

## Behavior Compatibility

- No intentional behavior changes.
- `cmd = "apply_settings"` route still executes through `HandleApplySettings(...)`.
- Existing parsing paths (`active`, `text`, `input_indicator`/`mouse_indicator`, `trail_*`, `theme`) are preserved.

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
