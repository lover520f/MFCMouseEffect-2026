# Trail Tuning UI (Preset + Advanced Parameters)

## What users get
In the Settings window (non-background mode), the **Trail** row now has a **Tuning...** button.
- Opens a dedicated window to tune trail behavior.
- Supports named presets and manual parameter edits.
- Changes are persisted to `config.json` and applied immediately.

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
- Electric: `fork_chance`, `amplitude_scale`
- Meteor: `spark_rate_scale`, `spark_speed_scale`

## Implementation
- UI window: `MFCMouseEffect/UI/Settings/TrailTuningWnd.cpp`
- Backend plumbing: `MFCMouseEffect/Settings/SettingsBackend.cpp`
- Persistence schema: `docs/architecture/trail-profiles-config.md`

