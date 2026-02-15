# Phase 13: CommandHandler Routing Table

## Summary

Refactored `CommandHandler::Handle(...)` from a long `if/else` chain to a command routing table:

- `cmd` string
- bound member handler

Goal: make command dispatch extension easier and reduce branching complexity in the main handler entry.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/CommandHandler.h` | Added explicit per-command private handler declarations |
| `MouseFx/Core/CommandHandler.cpp` | Added static command route table inside `Handle(...)`; moved each command path into dedicated methods |

### Refactored Commands

- `set_effect`
- `clear_effect`
- `set_theme`
- `set_ui_language`
- `effect_cmd`
- `reload_config`
- `reset_config`
- `apply_settings` (existing payload handler reused)

## Behavior Compatibility

- No intentional behavior changes.
- Command parsing keys and call order remain unchanged for each command route.
- `apply_settings` logic is unchanged and still delegated to `HandleApplySettings(...)`.

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
