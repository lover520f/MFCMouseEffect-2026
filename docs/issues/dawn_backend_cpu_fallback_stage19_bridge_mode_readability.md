# Dawn Backend + CPU Fallback (Stage 19 Bridge Mode Readability)

## Goal
- Improve user-facing readability of Dawn bridge mode in settings UI.
- Avoid exposing raw internal mode codes directly to end users.

## Changes

### 1) API Adds Bridge Mode Labels
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `dawn_overlay_bridge` payload now includes:
  - `mode_label_en`
  - `mode_label_zh`
- Current mappings:
  - `host_compat` -> `Host-Compatible Bridge` / `宿主兼容桥接`
  - `compositor` -> `GPU Compositor Bridge` / `GPU 合成桥接`
  - default -> `Not Enabled` / `未启用`

### 2) UI Uses Labels Instead of Raw Codes
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- GPU banner now prefers localized bridge labels from API payload.
- Fallback remains mode code if labels are absent.

## Result
- Settings banner is more understandable for end users.
- Internal implementation mode remains visible without requiring users to parse raw enum-like values.
