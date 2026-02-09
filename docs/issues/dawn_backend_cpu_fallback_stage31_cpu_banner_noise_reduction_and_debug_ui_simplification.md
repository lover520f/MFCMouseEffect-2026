# Dawn Backend + CPU Fallback (Stage 31 CPU Banner Noise Reduction + Debug UI Simplification)

## Goal
- Reduce noisy GPU warning text when user is intentionally using CPU path.
- Remove unnecessary MDI debug window and keep runtime behavior consistent.

## Changes

### 1) CPU Banner Noise Reduction
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `BuildGpuBannerJson(...)` now accepts backend preference.
- New behavior:
  - When backend preference is `cpu` and runtime is on CPU fallback:
    - banner shows a neutral informational message (manual CPU mode).
    - no aggressive Dawn DLL warning guidance.
  - When backend preference is `auto` and state is `loader_missing`:
    - banner text is softened to optional guidance.
    - action remains lightweight (`trigger_probe_now`) for future recheck.

### 2) Debug MDI Removal
- Updated:
  - `MFCMouseEffect/MFCMouseEffect.cpp`
- Debug mode no longer creates or shows `CMainFrame` MDI window.
- Debug/Release now both use hidden tray host window path, reducing useless debug UI surface.

## Result
- CPU-only users no longer see intrusive Dawn runtime missing warnings as primary UX.
- Debug startup is cleaner and closer to real runtime mode (tray-host driven).
