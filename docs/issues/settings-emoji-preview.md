# Settings Emoji Color Preview

## Summary
- RichEdit often renders emoji as monochrome on some Windows builds.
- We add a color preview overlay so the settings field matches the click effect.

## Approach
- Keep RichEdit for input and caret.
- Add a lightweight child window that renders the text with DirectWrite + Direct2D (HWND render target).
- Show preview only when the edit box is not focused to avoid caret conflicts.
- RichEdit keeps its normal text color while focused.
- Preview window returns `HTTRANSPARENT` so it never blocks clicks on the edit control.
- Preview font size uses `tmHeight - tmInternalLeading` to match the edit box metrics.

## Notes
- Text is drawn with `Segoe UI` for normal glyphs and `Segoe UI Emoji` for emoji ranges.
- The preview draws on its own window surface; background matches the edit control.
