# Trail Profiles in config.json (Hot Reload)

## Goal
Allow power users to tune strategy-based trail effects (**line / streamer / electric / meteor / tubes**) without rebuilding:
- Tail lifetime (`duration_ms`)
- History density (`max_points`)

And apply changes at runtime via a new IPC/tray command.

## JSON Schema
Root-level keys (optional):

### `trail_style`
Named preset label:
- `default | snappy | long | cinematic | custom`

### `trail_profiles`
Per-type history window:

```json
{
  "trail_style": "default",
  "trail_profiles": {
    "line":     { "duration_ms": 300, "max_points": 32 },
    "streamer": { "duration_ms": 420, "max_points": 46 },
    "electric": { "duration_ms": 280, "max_points": 24 },
    "meteor":   { "duration_ms": 520, "max_points": 60 },
    "tubes":    { "duration_ms": 350, "max_points": 40 }
  }
}
```

Notes:
- Missing keys fall back to built-in defaults.
- Values are sanitized:
  - `duration_ms`: clamped to `[80, 2000]`
  - `max_points`: clamped to `[2, 240]`
- `scifi` trail type is treated as alias of `tubes`.

### `trail_params`
Renderer-specific knobs (optional):

```json
{
  "trail_params": {
    "streamer": {
      "glow_width_scale": 1.8,
      "core_width_scale": 0.55,
      "head_power": 1.6
    },
    "electric": {
      "amplitude_scale": 1.0,
      "fork_chance": 0.10
    },
    "meteor": {
      "spark_rate_scale": 1.0,
      "spark_speed_scale": 1.0
    }
  }
}
```

## Hot Reload
## Config Location (Important)
- **Release**: `%AppData%\\MFCMouseEffect\\config.json` (preferred)
- **Debug**: `[ExeDir]\\config.json`
See also: `docs/issues/text_encoding_and_path_fix.md` (config path + encoding notes).

### Tray
Right-click tray icon → `Reload config`.

### Background mode (stdin JSON)
Send:
```json
{"cmd":"reload_config"}
```

## Implementation Map
- Parse/save config: `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`
- Profile lookup: `MFCMouseEffect/MouseFx/Core/EffectConfig.h`
- Apply profiles to trail effect: `MFCMouseEffect/MouseFx/Core/EffectFactory.cpp`
- Reload command: `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- Tray entry: `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
