# Stage 40 - Follow-Mode Copy Cleanup and FluxField GPU Label Unification

## Problem
- The hold follow-mode wording in Web settings was ambiguous:
  - `平滑跟随（推荐）`
  - `性能优先（省CPU）`
- User expectation: one option should clearly indicate cursor-priority behavior.
- FluxField GPU label used inconsistent phrasing:
  - `自动兜底CPU` / `Auto Fallback CPU`

## Root cause
- Follow-mode labels and helper tooltip text were generic and did not map directly to user mental model.
- FluxField GPU label text was duplicated in multiple layers and not normalized.

## Fix
1. Rename hold follow-mode labels:
   - `precise` -> `精准跟随（低延迟）` / `Precise (Low Latency)`
   - `smooth` -> `光标优先（推荐）` / `Cursor Priority (Recommended)`
   - `efficient` -> `性能优先（CPU友好）` / `Performance First (CPU Saver)`
2. Update hold follow-mode tooltip text in Web UI to match the new semantics.
3. Normalize FluxField GPU label:
   - `磁通场 HUD GPU（CPU兜底）` / `FluxField HUD GPU (CPU Fallback)`
4. Keep internal config values unchanged (`precise/smooth/efficient`) to preserve backward compatibility.

## Files changed
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `MFCMouseEffect/WebUI/app.js`
- `MFCMouseEffect/Settings/SettingsOptions.h`
- `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`

## Validation
1. Open Web settings and check hold follow-mode dropdown labels.
2. Hover follow-mode info icon and confirm tooltip wording is consistent.
3. Check hold effect list/tray labels for FluxField GPU and confirm `CPU兜底 / CPU Fallback`.
