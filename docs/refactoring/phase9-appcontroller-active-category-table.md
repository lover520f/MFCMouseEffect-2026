# Phase 9: AppController Active-Category Table

## Summary

Further reduced category-branch duplication in `AppController` by introducing a single active-category descriptor table:

- `EffectCategory`
- corresponding `EffectConfig::active` string slot

Goal: keep category routing logic centralized and remove repeated `switch`/hard-coded category lists.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/AppController.h` | Added `MutableActiveTypeForCategory` private helper declaration |
| `MouseFx/Core/AppController.cpp` | Added `kActiveCategoryDescriptors` table and reused it across active-type mutation/apply/normalize flows |

### Refactored Call Sites

- `SetActiveEffectType(...)`
  - before: `switch` by category
  - after: table-driven slot lookup via `MutableActiveTypeForCategory(...)`
- `ApplyConfiguredEffects()`
  - before: manual 5-category sequence
  - after: iterate descriptor table
- `NormalizeActiveEffectTypes()`
  - before: manual per-category calls
  - after: iterate descriptor table
- `ResetConfig()`
  - now reuses `ApplyConfiguredEffects()` (instead of manually reapplying 5 categories)

## Behavior Compatibility

- No intentional behavior changes.
- Click fallback order remains unchanged (`active.click` -> `defaultEffect` -> `"ripple"`).
- Theme-sensitive refresh behavior remains unchanged (`SetTheme` still selectively rebuilds categories).

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 error`
  - Existing warning remains: `C4819` in `MouseFx/Core/CommandHandler.cpp`
