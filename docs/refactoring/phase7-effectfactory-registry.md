# Phase 7: EffectFactory Registry Pivot

## Summary

Refactored `EffectFactory` from category-switch branching to a registry-driven creation table.

Goal: reduce branch growth when adding new effect types, and keep per-category creation rules explicit and localized.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/EffectFactory.cpp` | Replaced `switch + if` creation flow with static registry table (`typedCreators + fallbackCreator`) |

### New Creation Model

- `typedCreators`: explicit type -> creator mapping for strict categories.
- `fallbackCreator`: category-level generic creator for categories that intentionally accept dynamic types.

Current mapping:

- `Click`: `ripple`, `star`, `text` (typed)
- `Trail`: `particle` (typed) + generic `TrailEffect` fallback
- `Scroll`: `arrow`, `helix` (typed)
- `Hold`: generic `HoldEffect` fallback
- `Hover`: generic `HoverEffect` fallback
- `Edge`: no creator (unchanged)

## Behavior Compatibility

- `type == "none"` or empty still returns `nullptr`.
- Unknown type behavior remains category-consistent:
  - strict categories return `nullptr` (e.g. `click`, `scroll`)
  - generic categories use fallback (`trail`, `hold`, `hover`)

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
