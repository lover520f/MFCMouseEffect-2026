# Dawn Backend + CPU Fallback (Stage 1)

## Goal
- Prepare the runtime for Dawn GPU backend adoption without breaking current CPU rendering.
- Ensure deterministic fallback to CPU when GPU backend is unavailable.

## What Was Added

### 1) Config-Level Backend Preference
- Added `render_backend` to `EffectConfig`.
- Supported values:
  - `auto` (prefer Dawn, fallback CPU)
  - `dawn`
  - `cpu`
- Persisted via `config.json` load/save.

Files:
- `MFCMouseEffect/MouseFx/Core/EffectConfig.h`
- `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`

### 2) Runtime Backend Selection in OverlayHostService
- Added backend preference APIs:
  - `SetRenderBackendPreference(...)`
  - `GetRenderBackendPreference()`
  - `GetActiveRenderBackend()`
  - `IsGpuBackendAvailable(...)`
- Added runtime selection logic in `Initialize()`:
  - `auto` or `dawn`: try Dawn first
  - if Dawn unavailable/uninitialized: force CPU path
- Current Dawn path is intentionally a stub:
  - `TryInitializeDawnBackend()` returns false until real Dawn integration lands.
  - This keeps behavior stable and always falls back to CPU.

Files:
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`

### 3) AppController Wiring
- On startup, load `render_backend` from config and apply it to overlay host service.
- Added `SetRenderBackend(...)` command path:
  - persist config
  - switch backend preference
  - recreate active effects so new backend selection applies immediately
- Added command support:
  - `{"cmd":"set_render_backend","backend":"auto|dawn|cpu"}`
- Extended `/api/state` apply path to accept `render_backend`.

Files:
- `MFCMouseEffect/MouseFx/Core/AppController.h`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`

### 4) Web Settings Schema/State Exposure
- Exposed backend options in schema:
  - `render_backends`
- Exposed runtime state:
  - `render_backend`
  - `render_backend_active`
  - `dawn_available`

File:
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Behavior After This Change
- Default stays safe:
  - `render_backend = auto`
  - active backend currently resolves to CPU (because Dawn init is not yet wired).
- If user selects `dawn` and Dawn is unavailable:
  - runtime automatically falls back to CPU.

## Next Stage (Actual Dawn Integration)
- Implement `TryInitializeDawnBackend()`:
  - adapter/device creation
  - swapchain/surface binding with overlay window
  - command submission path
- Add a dedicated Dawn overlay renderer layer while keeping current CPU layer as fallback.
