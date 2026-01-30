# Tray/AppController Cleanup (2026-01-30)

## Goal
Keep `MouseFx/Renderers/*` (render implementation) intact, while reducing the “glue-code” feel in the app/controller + tray UI:
- Extract utility responsibilities out of `AppController`.
- Make tray menu table-driven (less duplication).
- Preserve behavior: tray still routes changes through `AppController::HandleCommand` for persistence.

## Summary
- Added `MouseFx/ConfigPathResolver.*` to centralize config directory selection.
- Added `MouseFx/EffectFactory.*` to centralize effect construction.
- Added `MouseFx/JsonLite.*` for minimal string-field extraction in small IPC-style JSON commands.
- Added `MouseFx/Renderers/RendererLinkage.cpp` to force static renderer registration linkage.
- Added `TrayMenuCommands.h` + `TrayMenuBuilder.*` and simplified `TrayHostWnd.cpp`.

## Manual Checks
1. Tray menu opens and reflects the current selected effects.
2. Switching click/trail/scroll/hold/hover updates immediately.
3. Switching theme persists to `config.json`.
4. Exit/settings actions behave as before.

