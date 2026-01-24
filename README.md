# MFCMouseEffect

Global mouse effect application with multi-category support.

## Effect Categories

| Category | Trigger | Available Effects |
|:---------|:--------|:------------------|
| **Click** | Mouse button release | ripple, star |
| **Trail** | Mouse movement | line |
| **Scroll** | Mouse wheel | (coming soon) |
| **Hover** | Mouse idle | (coming soon) |
| **Hold** | Button held down | (coming soon) |
| **Edge** | Mouse at screen edge | (coming soon) |

Each category can have one active effect. Multiple categories work simultaneously.

## Tray Menu

Right-click tray icon to access submenu per category with checkmarks.
For full configuration in tray mode, use “Settings...” in the tray menu.

## IPC Commands

```json
{"cmd": "set_effect", "category": "click", "type": "ripple"}
{"cmd": "set_effect", "category": "trail", "type": "line"}
{"cmd": "clear_effect", "category": "trail"}
{"cmd": "set_theme", "theme": "neon"}   // neon | scifi | minimal | game
{"cmd": "set_ui_language", "lang": "zh-CN"}  // zh-CN | en-US
{"cmd": "exit"}
```

Legacy format (assumes click category):
```json
{"cmd": "set_effect", "type": "ripple"}
```

## Launch Modes

```bash
MFCMouseEffect.exe              # Tray mode (default)
MFCMouseEffect.exe -mode tray   # Explicit tray mode
MFCMouseEffect.exe -mode background  # No tray icon
```

Background mode is IPC-only and will exit when stdin closes (parent process gone).

## Configuration

Place `config.json` next to the exe. Built-in defaults used if missing.

```json
{
  "default_effect": "ripple",
  "theme": "neon",
  "ui_language": "zh-CN",
  "active_effects": {
    "click": "ripple",
    "trail": "particle",
    "scroll": "arrow",
    "hover": "glow",
    "hold": "charge"
  },
  "effects": {
    "ripple": { "duration_ms": 350, "end_radius": 45 },
    "icon_star": { "end_radius": 35 },
    "trail": { "max_points": 20, "color": "#DC64FFDA" }
  }
}
```
