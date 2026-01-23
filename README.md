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

## IPC Commands

```json
{"cmd": "set_effect", "category": "click", "type": "ripple"}
{"cmd": "set_effect", "category": "trail", "type": "line"}
{"cmd": "clear_effect", "category": "trail"}
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

## Configuration

Place `config.json` next to the exe. Built-in defaults used if missing.

```json
{
  "default_effect": "ripple",
  "effects": {
    "click": {
      "ripple": { "duration_ms": 350, "end_radius": 45 },
      "star": { "end_radius": 35 }
    },
    "trail": {
      "line": { "max_points": 20, "color": "#DC64FFDA" }
    }
  }
}
```
