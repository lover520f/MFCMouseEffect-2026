# MFCMouseEffect

Language: [English](README.md) | [中文](README.zh-CN.md)

Windows global mouse click ripple effect (similar to `keyviz` mouse click visualization), implemented in MFC + Win32.

## What It Does
- Listens to global mouse clicks (left/right/middle) anywhere on Windows.
- Draws a short ripple animation at the click location.
- Uses a click-through layered window (does not block the underlying application).

## How To Build/Run
1. Open `F:\language\cpp\code\MFCMouseEffect\MFCMouseEffect.slnx` (or the `.sln` if you generated one) in Visual Studio.
2. Build and run the `MFCMouseEffect` project (x64 recommended).
3. When the app is running, click anywhere; you should see ripples.

## Architecture (Low Coupling)
The effect subsystem is isolated under `MFCMouseEffect\MouseFx\` and only exposed via a small controller:

- `MFCMouseEffect\MouseFx\AppController.*`
  - Owns lifecycle: initializes GDI+, creates a message-only dispatch window, starts/stops the global hook, and manages a pool of effect windows.
- `MFCMouseEffect\MouseFx\GlobalMouseHook.*`
  - Installs a `WH_MOUSE_LL` hook and posts click events to the dispatch window (keeps hook callback lightweight).
- `MFCMouseEffect\MouseFx\RippleWindow.*`
  - A per-click layered window that renders one ripple animation using GDI+ into a 32-bit ARGB DIB and `UpdateLayeredWindow`.
  - Enforces hit-test passthrough with `WM_NCHITTEST -> HTTRANSPARENT`.
- `MFCMouseEffect\MouseFx\RippleWindowPool.*`
  - Small pool to handle rapid clicks without allocating windows every time.
- `MFCMouseEffect\MouseFx\RippleStyle.h`
  - Central place for duration, sizes, and colors.

The MFC application layer only does:
- `mouseFx_ = std::make_unique<mousefx::AppController>(); mouseFx_->Start();`
- `mouseFx_->Stop();` on shutdown

## Customizing The Look
Edit `MFCMouseEffect\MouseFx\RippleStyle.h` and `MFCMouseEffect\MouseFx\RippleWindow.cpp`:
- Duration: `RippleStyle::durationMs`
- Ripple radius: `startRadius`, `endRadius`
- Window size: `windowSize` (larger looks softer but costs more GPU/CPU)
- Colors:
  - Left click: blue
  - Right click: orange
  - Middle click: green

## Notes / Known Constraints
- This is Windows-only (uses Win32 hooks and layered windows).
- Some protected/admin-only windows may behave differently if your process has lower privilege. If needed, run your app with the same integrity level.

## Documentation
More detailed build, run, customization, and troubleshooting notes:
- English: `docs/README.md`
- 中文: `docs/README.zh-CN.md`
