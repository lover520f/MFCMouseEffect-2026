# WASM Budget Policy Controls (Phase 3c)

## Summary

This increment makes WASM execution budget configurable and persisted, instead of hard-coded defaults.

Added policy fields:
- `output_buffer_bytes`
- `max_commands`
- `max_execution_ms`

All fields are persisted in `config.json`, reflected in settings state, and applied to runtime host budget.

## Architecture Changes

### 1. Config model and codec
- `WasmConfig` now contains budget fields.
- Added parse/serialize support for WASM budget keys.
- Added sanitization rules:
  - `output_buffer_bytes`: `1024..262144`
  - `max_commands`: `1..2048`
  - `max_execution_ms`: `0.1..20.0`

### 2. AppController policy application
- Added `SetWasmExecutionBudget(...)`.
- Added `ApplyWasmConfigToHost(bool tryLoadManifest)` for centralized host sync.
- Startup and reload now apply both policy and budget to host runtime.

### 3. Command/API bridge
- `wasm_set_policy` now accepts budget fields.
- `/api/wasm/policy` forwards budget updates.
- Numeric parsing is defensive against negative and overflow values.

### 4. Web settings panel
- WASM section now provides editable budget inputs and policy save action.
- Displays both configured values and runtime-applied budget snapshot.

## Validation

1. `pnpm run build` (WebUIWorkspace) passed.
2. `Release|x64` MSBuild passed.

## Notes

- Budget policy is intentionally host-enforced and clamped at config boundary.
- Runtime snapshot visibility helps distinguish "configured value" vs "effective host value".
