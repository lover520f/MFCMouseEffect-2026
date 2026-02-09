# Dawn Backend + CPU Fallback (Stage 11 Status Schema)

## Goal
- Stabilize `dawn_status` payload so frontend can rely on it long-term.
- Add explicit schema version and normalized state codes.

## Changes

### 1) Versioned `dawn_status`
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `BuildDawnStatusJson(...)` now outputs:
  - `schema_version` (current: `1`)
  - `state_code` (normalized enum-like value)
  - `detail` (resolved detail for compatibility/readability)
  - existing fields (`init_attempts`, `last_init_detail`, `last_init_tick_ms`, `ready_for_device_stage`, `probe`)

### 2) Stable State Code Mapping
- Added normalized mapper:
  - `DawnStateCodeFromDetail(...)`
- Supported state codes:
  - `init_not_run`
  - `no_display_adapter`
  - `build_disabled`
  - `loader_missing`
  - `symbols_missing`
  - `symbols_partial`
  - `create_proc_missing`
  - `create_failed`
  - `instance_ok_no_device`
  - `ready_for_device_stage`
  - `unknown`

### 3) Schema Declaration in `/api/schema`
- Added:
  - `gpu_status_schema.version`
  - `gpu_status_schema.state_codes`

## Result
- Frontend can pin behavior to `schema_version` and `state_code` without parsing ad-hoc detail strings.
- Backward compatibility is retained via existing detail/legacy fields.
