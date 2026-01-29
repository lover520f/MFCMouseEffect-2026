# Emoji Support in Text Click Effect

## Summary
- Goal: allow emoji like "💰" and "😊" to display correctly in both the settings input box and the click text effect.
- Scope: settings edit control + text rendering in `TextWindow`.

## Root Cause
- The settings input used a plain Win32 `EDIT` control (GDI-based), which does not render color emoji and often falls back to monochrome glyphs.
- The click text effect used GDI+ with a font family that does not contain emoji glyphs, so emoji rendered as missing/outline.

## Fix
- **Settings window**: switched the text input to a RichEdit 4.1 control (`RICHEDIT50W`) so DirectWrite can handle emoji rendering and font fallback.
  - Force RichEdit CTF and use Segoe UI Emoji as the edit base font for better emoji rendering.
  - Apply per-character font formatting: default Segoe UI, emoji ranges use Segoe UI Emoji.
- **Text effect**: detect emoji-only strings and switch the font family to `Segoe UI Emoji` for those cases, with a safe fallback to `Segoe UI`.
  - Emoji-only strings now use a regular font style (not bold) to avoid missing glyphs in emoji fonts.
  - Comma-separated tokens are trimmed to avoid leading spaces preventing emoji detection.
  - Mixed text now splits runs and renders emoji with Segoe UI Emoji.
  - Rendering switched to DirectWrite + Direct2D to enable color emoji in click effects.
  - DWrite font size treats config font_size as points and converts to DIPs (96/72) to match GDI+ default units.
  - D2D render target uses window DPI; layout sizes convert pixels to DIPs for consistent text scale on high DPI.

## Notes
- Mixed text (CJK + emoji) still uses the configured font to preserve Chinese glyphs. Emoji-only tokens are drawn with the emoji font.
- RichEdit is initialized via `AfxInitRichEdit2()` and `Msftedit.dll` to ensure the 4.1 control is available.

## Manual Test
1. Open Settings window.
2. In text content, input `💰,😊` and click Apply.
3. Click anywhere: emojis should appear (not as outline boxes).
4. Optional: add a Chinese word and an emoji as separate tokens to confirm both render correctly.
