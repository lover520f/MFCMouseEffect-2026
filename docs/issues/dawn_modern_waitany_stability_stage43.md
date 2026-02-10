# Dawn Modern WaitAny Stability (Stage 43)

## Goal
- Improve modern ABI handshake robustness on runtimes that may return non-success wait statuses during callback progress.
- Reduce false negative probe results like `modern_request_adapter_wait_status_3` / `modern_request_device_status_3`.

## Changes
### 1) WaitAny callback mode alignment
- File: `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- In modern request flows:
  - `SafeRequestAdapterCallModern(...)`
  - `SafeRequestDeviceCallModern(...)`
- Callback mode is now retried with compatible fallback chain:
  - `WaitAnyOnly` -> `AllowProcessEvents` -> `AllowSpontaneous`
- This avoids hard-coding one callback mode for all Dawn runtime variants.

### 2) Non-fatal wait status handling
- In both modern request flows, when `waitAny` returns a status other than `success/timedout`:
  - no immediate hard-fail if `processEvents` is available;
  - continue `processEvents + poll` until deadline;
- only fail when timeout or explicit exception occurs.

### 3) Adapter/device paired retry (avoid consumed adapter)
- Modern mode retries are now done as paired attempts:
  - per callback mode, request a fresh adapter, then request device from that adapter.
- This avoids reusing one adapter across multiple device retries.
- Fixes runtime failures like:
  - `modern_request_device_status_3_msg: adapter is "consumed"...`

### 4) Callback completion window
- Final callback wait timeout increased:
  - from `50ms` to `600ms`
- This avoids false `callback_timeout` under slower callback scheduling.

### 5) Diagnostics
- Added success detail marker:
  - `ok_wait_status_fallback`
- Helps confirm runtime completed via fallback polling path.

## Expected Effect
- Better tolerance for modern-runtime wait status jitter.
- Fewer premature fallbacks to CPU caused by transient wait status anomalies.
- Does not change CPU fallback safety behavior when real adapter/device failure occurs.
