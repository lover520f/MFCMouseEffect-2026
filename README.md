# MFCMouseEffect

Application for global mouse click effects (ripples).

## Launch Modes

The application supports two launch modes via command line arguments:

### 1. Tray Mode (Default)
Shows a system tray icon for management (Exit option).
```bash
MFCMouseEffect.exe
# OR
MFCMouseEffect.exe -mode tray
```

### 2. Background Mode
Runs fully in the background without any tray icon. Useful when managed by a parent process.
```bash
MFCMouseEffect.exe -mode background
```
In this mode, the parent process is responsible for terminating the application.

## Inter-Process Communication (IPC) And Commands

The application listens for commands via **Standard Input (Stdin)**. This allows parent processes to control the application (e.g., change effects) by writing JSON strings to the process's stdin pipe.

**Protocol**: One JSON object per line.

### Supported Commands

#### Switch Effect
```json
{"cmd": "set_effect", "type": "ripple"}
{"cmd": "set_effect", "type": "none"}
```

## Features
- **Global Mouse Hook**: Intercepts mouse clicks system-wide.
- **Efficient Rendering**: Uses GDI+ and Layered Windows for transparent overlays.
- **Architecture**: Separated Core Logic (`AppController`) and Effect Implementations (`IMouseEffect`).
