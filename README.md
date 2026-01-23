# MFCMouseEffect

Application for global mouse click effects (ripples, trails, icons).

## Configuration

The application loads settings from `config.json` in the same directory as the executable.
If the file is missing, built-in defaults are used.

### Sample `config.json`
```json
{
    "default_effect": "ripple",
    "effects": {
        "ripple": {
            "duration_ms": 350,
            "end_radius": 45,
            "left_click": { "fill": "#594FC3F7", "stroke": "#FF0288D1" }
        },
        "trail": {
            "duration_ms": 350,
            "max_points": 20,
            "color": "#DC64FFDA"
        },
        "icon_star": {
            "end_radius": 35,
            "fill": "#FFFFD700",
            "stroke": "#FFFF8C00"
        }
    }
}
```

## Available Effects

| Name | Description |
|:-----|:------------|
| `ripple` | Expanding circle on click (default) |
| `trail` | Fading line following cursor movement |
| `icon_star` | Star icon splash on click |
| `none` | Disable all effects |

## Launch Modes

### 1. Tray Mode (Default)
Shows a system tray icon for management.
```bash
MFCMouseEffect.exe
# OR
MFCMouseEffect.exe -mode tray
```

### 2. Background Mode
Runs fully in the background without tray icon.
```bash
MFCMouseEffect.exe -mode background
```

## IPC Commands

The application listens for JSON commands via **Stdin**.

```json
{"cmd": "set_effect", "type": "ripple"}
{"cmd": "set_effect", "type": "trail"}
{"cmd": "set_effect", "type": "icon_star"}
{"cmd": "set_effect", "type": "none"}
```

