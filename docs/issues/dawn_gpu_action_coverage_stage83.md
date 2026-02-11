# Dawn GPU action coverage in Web settings (Stage 83)

## Background
- Web settings receives normalized GPU suggestion actions from backend banner (`gpu_status_banner.action.action_code`).
- Frontend only recognized part of the action set, so some valid actions were hidden in UI.

## Problem
- Action button visibility depended on `isGpuActionSupported(actionCode)`.
- Codes like `install_dawn_runtime`, `replace_runtime_binary`, `enable_dawn_build_flag`, `check_display_adapter` were not included.
- For these states, users could not trigger the default "recheck/probe" workflow from the same UI action area.

## Changes
- Updated `MFCMouseEffect/WebUI/app.js`:
  - Expanded `isGpuActionSupported(...)` to include:
    - `install_dawn_runtime`
    - `replace_runtime_binary`
    - `enable_dawn_build_flag`
    - `check_display_adapter`
  - Existing execution path remains backward-compatible:
    - Special actions still keep dedicated handling.
    - Other supported actions fall back to probe/recheck flow.

## Effect
- Action button now remains visible for all backend-provided, user-relevant guidance codes.
- GPU troubleshooting path stays continuous in Web settings without introducing extra APIs.

