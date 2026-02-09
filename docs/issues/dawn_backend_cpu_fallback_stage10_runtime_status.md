# Dawn Backend + CPU Fallback (Stage 10 Runtime Status)

## Goal
- Add a stable runtime-status snapshot for Dawn initialization attempts.
- Expose probe + init-attempt metrics together for better diagnostics.

## Changes

### 1) DawnRuntime Status Model
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added:
  - `DawnRuntimeStatus`
    - `initAttempts`
    - `lastInitDetail`
    - `lastInitTickMs`
    - `readyForDeviceStage`
    - embedded `probe`
  - `GetDawnRuntimeStatus()`
- Internal counters now track every `TryInitializeDawnRuntime()` call.

### 2) Thread-Safe Status Accounting
- Reused existing probe mutex to guard:
  - init attempt counter
  - last init detail/timestamp
  - probe state
- `ResetDawnRuntimeProbe()` also resets init counters.

### 3) Web Payload Expansion
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added helper:
  - `BuildDawnStatusJson(...)`
- API output updates:
  - `/api/state` now includes `dawn_status`
  - `/api/gpu/probe_now` now includes `dawn_status`

## Result
- Runtime diagnostics now show:
  - probe capability snapshot
  - initialization behavior over time
  - whether current state is ready for next device stage
- This improves observability before adapter/device integration.
