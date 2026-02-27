# Phase 56n - Effects Normalization Single Source and Active Alignment

## Why
- Effect type normalization still had multi-source drift:
  - `EffectConfig::GetTrailHistoryProfile` used local trail alias logic.
  - macOS render profile helper used another local trail alias logic.
  - `AppController::ResolveRuntimeEffectType` only normalized hold category.
  - macOS factory path could receive raw active type and rely on downstream fallback behavior.
- This increases risk of `active` type not matching effective runtime behavior.

## What Changed
1. Unified trail normalization source:
- `NormalizeTrailEffectType` now also recognizes `scifi/sci-fi/sci_fi -> tubes`.
- `EffectConfig::GetTrailHistoryProfile` now uses `NormalizeTrailEffectType`.
- `MacosEffectRenderProfile.Shared` trail alias normalization now delegates to `NormalizeTrailEffectType`.

2. Unified runtime active/effective normalization in controller:
- `AppController::ResolveRuntimeEffectType` now normalizes all categories:
  - click -> `NormalizeClickEffectType`
  - trail -> `NormalizeTrailEffectType`
  - scroll -> `NormalizeScrollEffectType`
  - hover -> `NormalizeHoverEffectType`
  - hold -> `NormalizeHoldEffectType` (+ route reason)

3. Factory input alignment:
- `EffectFactory::Create` now normalizes requested type for all platforms before dispatch.
- macOS factory branch now consumes normalized type (same as Windows input contract).

4. Contract observability expansion:
- `/api/effects/test-render-profiles` `command_samples.sample_input` now includes:
  - `active_raw`
  - `active_normalized`
- alias matrix extended with additional normalization samples:
  - trail: `scifi -> tubes`
  - scroll: `none -> arrow`
  - hover: `none -> glow`

## Risks
- Runtime behavior now consistently canonicalizes non-hold categories; this is intended and matches existing compute semantics.
- Unknown types continue to degrade to category defaults (not null effect), preserving existing fallback safety.

## Validation
```bash
./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto
```
