# Trail Tuning UI (Preset + Advanced Parameters)

## Status (Feb 2026)
This MFC “Trail Tuning” window is now **legacy/deprecated**. The tray **Settings...** entry opens the browser-based web settings UI instead:
- `docs/architecture/web-settings-ui.md`

## What users get
In the Settings window (non-background mode), the **Trail** row now has a **Tuning...** button.
- Opens a dedicated window to tune trail behavior.
- Supports named presets and manual parameter edits.
- Changes are persisted to `config.json` and applied immediately.
 - Uses the same custom header chrome as the main settings window (no default caption bar).

## Presets
Built-in presets:
- `Default`
- `Snappy`
- `Long`
- `Cinematic`
- `Custom` (any manual edits)

The preset label is stored in `trail_style`.

## Parameters exposed
### History (per trail type)
- `duration_ms`
- `max_points`

### Renderer params
- Streamer: `glow_width_scale`, `head_power`
- Streamer: `core_width_scale`
- Electric: `fork_chance`, `amplitude_scale`
- Meteor: `spark_rate_scale`, `spark_speed_scale`

## Implementation
- UI (legacy): `MFCMouseEffect/UI/Settings/TrailTuningWnd.Core.cpp`
- UI chrome (legacy): `MFCMouseEffect/UI/Settings/TrailTuningWnd.Chrome.cpp`
- UI model (legacy): `MFCMouseEffect/UI/Settings/TrailTuningWnd.Model.cpp`
- Backend plumbing: `MFCMouseEffect/Settings/SettingsBackend.cpp`
- Persistence schema: `docs/architecture/trail-profiles-config.md`
