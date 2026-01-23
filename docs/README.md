# MFCMouseEffect Documentation

Language: [English](README.md) | [中文](README.zh-CN.md)

## What It Is
- Global mouse click visualization for Windows: low-level hook (`WH_MOUSE_LL`) + GDI+ layered ripple windows.
- Click-through: does not block the underlying app.

## Build & Run
1. Open `MFCMouseEffect.slnx` (or the generated `.sln`) in Visual Studio.
2. Choose `x64` + `Debug` (recommended for testing).
3. Build and run `MFCMouseEffect` — output goes to `x64\Debug\MFCMouseEffect.exe` (Release: `x64\Release\...`).
4. On launch (Debug), a one-shot self-test ripple should appear at the cursor within ~250 ms. Then every mouse click should ripple.
5. If clicking admin/elevated windows, run the app as Administrator for matching integrity.

## Release: Tray-Only (No Window)
Release builds no longer create the main frame window. A hidden host window is used solely for the tray icon, so there is no startup flash.
- Entry: `x64\Release\MFCMouseEffect.exe`
- Tray icon: right-click menu “Exit” (double-click also exits)
- Debug still shows a main window for convenience

## Customizing the Look
- File: `MFCMouseEffect/MouseFx/RippleStyle.h` and `MFCMouseEffect/MouseFx/RippleWindow.cpp`.
- Key knobs:
  - Duration: `RippleStyle::durationMs`
  - Radii: `startRadius`, `endRadius`
  - Window size: `windowSize`
  - Colors per button: `RippleWindow::StartAt(...)` switch (fill/stroke/glow)

## Troubleshooting
- **No ripple at all (Debug):** the self-test ripple did not show. Start likely failed. Check the dialog for `Stage/Error/Message`.
  - Stage `dispatch window`, Error `1400 (invalid window handle)`: fixed in code; rebuild and run `x64\Debug\MFCMouseEffect.exe`.
  - Other errors: see the error text; often permissions or system policies.
- **Hook errors:** The dialog or VS Output window prints `MouseFx: global hook start failed. GetLastError=...`. If you click elevated windows, run this app elevated. Security software may also block hooks.
- **Ripple off-position at >100% DPI:** DPI awareness is enabled at startup; rebuild and run the newest binary.
- **Running wrong binary:** There was a duplicate output under `MFCMouseEffect\x64\Debug\...`. Current project outputs to `x64\Debug\...`. Clean + Rebuild to ensure you run the right one.

## SDI / Single-Window Notes
- The app now uses an SDI frame: one top-level window hosts the view; ripples are still rendered in separate transparent layered windows, so UI and effect remain decoupled.
- If you need multiple windows, create multiple top-level frames (no MDI children/tabs), or run multiple instances.
