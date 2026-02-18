# WASM Emoji Text Fallback Routing (2026-02)

## Background
- When a WASM plugin outputs mixed `CJK + emoji` text, emoji could render as tofu squares.
- Built-in click text effect usually renders emoji correctly, so behavior was inconsistent.

## Root Cause
- `WASM spawn_text` goes through `OverlayHostService::ShowText` into `TextOverlayLayer` (GDI+ path).
- Built-in click text switches to `TextWindowPool` for emoji (DirectWrite fallback path).
- The mismatch caused WASM text to miss the emoji-friendly render route.

## Fix
- Unified routing in `OverlayHostService::ShowText`:
  - If text contains emoji, render via shared `TextWindowPool`.
  - Otherwise keep using `TextOverlayLayer`.
- Added pool cleanup in `OverlayHostService::Shutdown` to avoid stale resources.

## Files
- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.cpp`

## Validation
1. Use a WASM plugin that emits mixed text like `财富 + 😄 + 🪙`.
2. Trigger click and verify emoji renders instead of tofu boxes.
3. Recheck normal text rendering to confirm no regressions.

