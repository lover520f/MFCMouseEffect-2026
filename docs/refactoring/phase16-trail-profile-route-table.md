# Phase 16: Trail-Profile Route Table

## Summary

Replaced hard-coded `applyProfile(...)` calls for trail profile keys in `HandleApplySettings(...)` with a small route table loop.

Goal: keep `trail_profiles` key-to-target mapping centralized and avoid repeated manual calls.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/CommandHandler.cpp` | Added `trailProfileRoutes` table (`key` + destination profile pointer) and iterated it for profile application |

### Mapping Covered

- `line` -> `profiles.line`
- `streamer` -> `profiles.streamer`
- `electric` -> `profiles.electric`
- `meteor` -> `profiles.meteor`
- `tubes` -> `profiles.tubes`

## Behavior Compatibility

- No intentional behavior changes.
- Field clamping and `trailTouched` behavior remain unchanged.
- Processing order remains equivalent to previous manual sequence.

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
