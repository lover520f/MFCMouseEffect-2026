# Dawn Backend + CPU Fallback (Stage 12 UI Awareness + Actions)

## Goal
- Make GPU/CPU runtime mode clearly visible in settings UI.
- Provide actionable guidance when Dawn is not active.

## Changes

### 1) Backend State Extensions (`/api/state`, `/api/gpu/probe_now`)
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added fields:
  - `gpu_in_use` (bool, true when active backend is `dawn`)
  - `gpu_status_banner`:
    - `code`
    - `tone`
    - `text_en` / `text_zh`
    - `state_code`
    - `action`:
      - `action_code`
      - `action_text_en` / `action_text_zh`
- Added normalized action mapping from state code:
  - `wire_device_stage`
  - `install_dawn_runtime`
  - `replace_runtime_binary`
  - `validate_runtime_abi`
  - `enable_dawn_build_flag`
  - `check_display_adapter`
  - `trigger_probe_now`
  - `review_logs`

### 2) Schema Extension (`/api/schema`)
- Added `gpu_status_schema.action_codes` for frontend compatibility.

### 3) Settings UI Banner
- Updated UI files:
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUI/styles.css`
  - `MFCMouseEffect/WebUI/app.js`
- Added top-level GPU status banner:
  - Shows `[GPU]` when Dawn backend is active.
  - Shows `[CPU]` fallback with remediation text when GPU path is unavailable.
- Banner updates on:
  - initial load
  - periodic connection probe
  - language switch

### 4) Manual Probe Interaction Fix
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Fixed:
  - `renderGpuBanner` previously wrote to the whole container via `textContent`, which removed child nodes (including the `Recheck GPU` button).
- Added:
  - dedicated text target update (`#gpuBannerText`) without destroying button nodes.
  - `btn_probe_gpu` i18n text (en/zh).
  - button click flow: `POST /api/gpu/probe_now` with `refresh=true`.
  - status feedback strings for probe progress/success/failure.

## Result
- Users can now directly perceive whether GPU is currently active.
- CPU fallback is no longer silent; users receive concrete next-step guidance.

## Build Compatibility Fix (GBK/936)
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Problem:
  - Direct non-ASCII Chinese literals in narrow C++ strings caused `C4819` and could cascade into parser errors (`C2001/C2064/C2059`) under code page 936 builds.
- Fix:
  - Replaced Chinese literals with ASCII-safe `u8"\uXXXX"` escaped UTF-8 string literals.
- Impact:
  - Keeps Chinese UI text output intact while avoiding source-encoding dependent compile failures.
