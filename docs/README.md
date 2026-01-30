# MFCMouseEffect Documentation

Language: [English](README.md) | [中文](README.zh-CN.md)

## Doc Index
- Marketing: `docs/marketing/readme_language_switch.md` (README language toggle rendering)
- Issues: `docs/issues/emoji-support.md` (emoji rendering in settings + text click effect)
- Issues: `docs/issues/text-effect-motion.md` (floating text motion parity after DWrite switch)
- Issues: `docs/issues/settings-emoji-preview.md` (settings emoji color preview overlay)
- Architecture: `docs/architecture/tray-and-appcontroller-refactor.md` (tray menu table-driven + AppController cleanup)
- Architecture: `docs/architecture/settingswnd-emoji-split.md` (SettingsWnd emoji logic split)
- Architecture: `docs/architecture/ui-folder-structure.md` (UI folder layout refolder)
- Architecture: `docs/architecture/mousefx-folder-structure.md` (MouseFx folder layout refolder)

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

## Settings Window (Non-background Mode)
Tray menus are fine for quick toggles, but not great for full configuration. In non-background mode, a lightweight settings window can be opened from the tray menu to configure theme + per-category effects, with persistence to `config.json`.
Closing the window (X) only closes the settings UI; it does not stop the tray/background process.
Language: the settings window supports Chinese/English switching (default Chinese) and persists to `config.json` (`ui_language`).
Theme: the selected theme affects the overall look of click/scroll/hold/hover effects (non-text effects).
Look: the settings window draws a custom frame/title and is decoupled from effect logic via a backend interface.
Note: inner controls use native widgets for stability.
Colors: no card/background fill; window and controls use system defaults, with only the title area and close button drawn.

## Customizing the Look
- File: `MFCMouseEffect/MouseFx/Styles/RippleStyle.h` and `MFCMouseEffect/MouseFx/Windows/RippleWindow.cpp`.
- Key knobs:
  - Duration: `RippleStyle::durationMs`
  - Radii: `startRadius`, `endRadius`
  - Window size: `windowSize`
  - Colors per button: `RippleWindow::StartAt(...)` switch (fill/stroke/glow)

## Operational Notes (Macro-Level)
- **UAC / admin windows:** hooks may not fully work inside elevated apps unless this app is also run elevated.
- **Tray vs background mode:** tray mode is interactive; background mode is IPC-only (no tray UI).
- **IPC control:** background mode is designed to be controlled by a parent process via stdin JSON; it exits when stdin closes.
- **Persistence:** `config.json` lives next to the exe; theme and active effects are persisted there when changed via tray/IPC.
- **Security software:** some endpoint/security tools may block global hooks or layered windows.

## Troubleshooting
- **No ripple at all (Debug):** the self-test ripple did not show. Start likely failed. Check the dialog for `Stage/Error/Message`.
  - Stage `dispatch window`, Error `1400 (invalid window handle)`: fixed in code; rebuild and run `x64\Debug\MFCMouseEffect.exe`.
  - Other errors: see the error text; often permissions or system policies.
- **Hook errors:** The dialog or VS Output window prints `MouseFx: global hook start failed. GetLastError=...`. If you click elevated windows, run this app elevated. Security software may also block hooks.
- **Ripple off-position at >100% DPI:** DPI awareness is enabled at startup; rebuild and run the newest binary.
- **Running wrong binary:** There was a duplicate output under `MFCMouseEffect\x64\Debug\...`. Current project outputs to `x64\Debug\...`. Clean + Rebuild to ensure you run the right one.
- **Virtual secondary display offset:** some tablet/virtual display drivers can cause coordinate-space mismatch (DPI mapping). See: `docs/issues/virtual-display-coordinates.md`.
  - Jan 2026: coordinate normalization is enabled by default; most virtual/ tablet display offsets should now be gone. If it still offsets, please report with driver/app name and DPI settings.

## SDI / Single-Window Notes
- The app now uses an SDI frame: one top-level window hosts the view; ripples are still rendered in separate transparent layered windows, so UI and effect remain decoupled.
- If you need multiple windows, create multiple top-level frames (no MDI children/tabs), or run multiple instances.
