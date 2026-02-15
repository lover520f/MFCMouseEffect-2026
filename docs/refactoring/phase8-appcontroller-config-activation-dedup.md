# Phase 8: AppController Config-Activation Dedup

## Summary

Deduplicated `AppController` logic for:

- resolving effective click type from config/default,
- applying active effects from current config,
- normalizing runtime-resolved active effect ids.

Goal: keep startup and reload behavior consistent while removing duplicated control flow.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/AppController.h` | Added private helper declarations: `ResolveConfiguredClickType`, `ApplyConfiguredEffects`, `NormalizeActiveEffectTypes` |
| `MouseFx/Core/AppController.cpp` | Implemented the helpers and reused them in `Start()` and `ReloadConfigFromDisk()` |

### Deduplicated Flows

- `Start()` now calls:
  - `ApplyConfiguredEffects()`
  - `NormalizeActiveEffectTypes()` + `PersistConfig()` on change
- `ReloadConfigFromDisk()` now follows the same sequence.

This removes repeated blocks and keeps the two lifecycle paths aligned.

## Behavior Compatibility

- No intentional behavior change.
- Click effect fallback order remains:
  1. `config_.active.click`
  2. `config_.defaultEffect`
  3. `"ripple"`

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 error`
  - Existing warning remains: `C4819` in `MouseFx/Core/CommandHandler.cpp` (pre-existing encoding issue)
