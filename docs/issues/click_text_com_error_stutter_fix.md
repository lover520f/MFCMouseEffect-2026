# Click Text: `_com_error` spam and first-click hitch

## Symptom

- In Visual Studio debug output, left click can trigger repeated first-chance `_com_error`.
- First left click can visibly hitch, while later clicks are smoother.
- Other effects (trail/hover/hold) do not show the same behavior.

## Root Cause

- Emoji click text uses legacy `TextWindow` (D2D/DWrite path), not the host GDI+ layer.
- Color-font rendering (`D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT`) can produce heavy COM first-chance noise on some systems.
- Emoji window pool was created lazily on first emoji click, so startup cost was paid on interaction.

## Fix

### 1) Stabilize DWrite draw path

- File: `MFCMouseEffect/MouseFx/Windows/TextWindow.cpp`
- Change `DrawTextLayout` option to `D2D1_DRAW_TEXT_OPTIONS_NONE` for stable behavior under debugger.

### 2) Remove first-click pool cold start

- File: `MFCMouseEffect/MouseFx/Effects/TextEffect.cpp`
- During `Initialize()`, prewarm emoji pool only when configured texts contain emoji.
- Reduce pool size from `15` to `8` to lower one-time cost while keeping enough concurrency.

## Verification

1. Start app with emoji text-click content enabled.
2. First left click should no longer show a visible hitch from pool creation.
3. Visual Studio output should not flood with click-driven `_com_error` at animation frame rate.
4. Emoji click text still renders and animates.

## Notes

- This fix is intentionally stability-first for debug/dev sessions.
- If future requirement is full color emoji fidelity with zero first-chance noise, consider replacing legacy emoji path with a host-side text renderer that supports color glyph runs.

## 2026-02-08 Regression Guard

- Symptom returned when `TextWindow::RenderFrame()` switched back to `D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT`.
- Resolution: force `D2D1_DRAW_TEXT_OPTIONS_NONE` again in `TextWindow.cpp` to keep debugger output and frame pacing stable.

## 2026-02-08 Additional Source: Settings Emoji Preview

- Even after stabilizing `TextWindow`, first-chance `_com_error` can still flood when settings UI keeps repainting emoji preview.
- Root cause: `UI/Settings/EmojiPreviewWnd.cpp` still used `D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT`.
- Resolution:
  - Switch preview draw option to `D2D1_DRAW_TEXT_OPTIONS_NONE`.
  - Add a defensive early return when `CreateTextFormat(...)` fails, instead of continuing with a null format.

## 2026-02-08 Final Stabilization (Revised Again): Emoji Cached-Frame Path

- Requirement conflict: host-first avoids COM spam but cannot preserve full-color emoji; legacy DWrite preserves color but can spam `_com_error` when color-font draw runs every frame.
- Final route:
  - Emoji text -> `TextWindowPool`
  - Non-emoji text -> host overlay first, pool fallback
- `TextWindow` emoji rendering strategy changed:
  - Render color emoji glyphs once with `D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT`.
  - Reuse cached backbuffer for subsequent animation frames (position + alpha only), avoiding per-frame color-font draw calls.
- This keeps original color emoji appearance while reducing COM first-chance noise significantly.
