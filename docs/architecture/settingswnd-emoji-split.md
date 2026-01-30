# SettingsWnd Emoji Split (2026-01-30)

## Goal
Reduce the “mixed responsibilities” feel in `SettingsWnd.cpp` with a minimal, non-overdesigned extraction:
- Keep UI/layout/paint logic intact.
- Move emoji parsing + rich-edit formatting + preview sync to dedicated units.

## Changes
- Added `MFCMouseEffect/Settings/EmojiUtils.*` for UTF-16 codepoint iteration and emoji detection helpers.
- Added `MFCMouseEffect/UI/Settings/SettingsWnd.Emoji.cpp` holding text-box handlers and emoji formatting/preview sync.
- Removed the inline emoji helper functions from `MFCMouseEffect/UI/Settings/SettingsWnd.cpp`.

## Manual Checks
1. Typing emojis still formats segments with `Segoe UI Emoji`.
2. Emoji preview behavior remains the same on focus/blur.
