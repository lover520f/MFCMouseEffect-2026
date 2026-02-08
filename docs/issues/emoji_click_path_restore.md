# Emoji click path restore

## Symptom

- Emoji text click no longer renders after recent click-text routing changes.

## Root cause

- `TextEffect` was changed to host-first for all text.
- The host text layer does not fully replace the legacy emoji-specific path behavior.

## Fix

- File: `MFCMouseEffect/MouseFx/Effects/TextEffect.cpp`
- Restore emoji detection helpers:
  - `NextCodePoint`
  - `IsEmojiCodePoint`
  - `HasEmojiStarter`
- Routing rule:
  - Emoji text -> `TextWindowPool`
  - Non-emoji text -> host text layer first, pool fallback.
  - Emoji legacy path now uses cached-frame rendering: color-font layout is drawn once, later frames only move/fade the cached buffer.
- Keep prewarm behavior:
  - Prewarm emoji pool when configured texts include emoji.
  - Initialize host service eagerly as best effort.
- File: `MFCMouseEffect/MouseFx/Windows/TextWindow.cpp`
  - (Updated for stability) use `D2D1_DRAW_TEXT_OPTIONS_NONE` to avoid debugger `_com_error` flood on some systems.
  - Emoji still routes through `TextWindowPool`; this change is render-option stability, not path removal.

## Validation

1. Configure click text list with emoji and plain text mixed.
2. Click repeatedly.
3. Confirm emoji keeps native color glyph rendering.
4. Confirm plain text rendering remains unchanged.
