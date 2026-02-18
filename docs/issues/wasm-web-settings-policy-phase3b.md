# WASM Settings Policy Controls (Phase 3b)

## Summary

This increment completes the first policy-control slice for WASM in Web settings:
- policy is now persisted in `config.json`,
- startup behavior is driven by persisted WASM config,
- click fallback behavior is user-configurable.

## Implemented Policies

Added `wasm` config section:
- `enabled`: whether WASM host is enabled after startup
- `fallback_to_builtin_click`: when WASM is active but does not render, whether to fallback to built-in click effect
- `manifest_path`: selected plugin manifest path for startup restore

## Runtime Behavior Changes

1. Startup
- `AppController::InitializeWasmHost()` now applies persisted WASM config:
  - sanitize config
  - load `manifest_path` when provided
  - apply persisted `enabled`

2. Click dispatch policy
- `DispatchRouter` now checks persisted fallback policy:
  - if WASM route is active and policy is disabled, built-in click fallback is suppressed
  - if WASM route is inactive, built-in click remains unchanged

3. Command/API policy updates
- New command: `wasm_set_policy`
- New API: `POST /api/wasm/policy`
- Existing WASM enable/disable/load-manifest now persist config via controller methods

## Config Codec Changes

`EffectConfig` now includes `WasmConfig` and JSON codec support:
- parse: `EffectConfigJsonCodec.Parse.Wasm.cpp`
- serialize: `EffectConfigJsonCodec.Serialize.Wasm.cpp`
- keys: `EffectConfigJsonKeys.Wasm.h`

## Web Settings UX

WASM panel now includes:
- persisted enabled indicator
- `fallback_to_builtin_click` toggle + save action
- configured manifest path display

## Verification

1. Frontend build
- `pnpm run build` in `MFCMouseEffect/WebUIWorkspace` passed.

2. Native build
- `Release|x64` MSBuild passed with new files and policy routing.

## Risks

- If fallback policy is disabled and plugin outputs empty commands, clicks may show no visual effect by design.
- Persisted `manifest_path` may become stale; startup load fails gracefully and keeps diagnostics.
