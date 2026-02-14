# Virtual / Tablet Secondary Display: Cursor Coordinate Offset

## Status
- Fixed (Jan 2026): coordinate normalization now keeps effects aligned even on most virtual/tablet secondary displays. If you still see offsets, capture logs with the driver name and DPI scaling.

## Symptoms
- When using a tablet as a secondary display via a "virtual display" app/driver:
  - click/scroll/trail/hover effects are visibly offset
  - effects may appear on the wrong monitor or "disappear" (drawn off-screen)
- This may or may not reproduce on a real external monitor.

## Likely Causes (High-Level)
- Virtual display drivers/apps can implement coordinate and DPI mapping differently.
- Coordinate-space mismatch between:
  - `MSLLHOOKSTRUCT.pt` from `WH_MOUSE_LL`
  - `GetCursorPos()`

## Mitigation Implemented
- Input point resolution is now cursor-first:
  - Prefer `GetCursorPos()`
  - Fallback to hook point (`MSLLHOOKSTRUCT.pt`) only when cursor query fails.
- This keeps point selection consistent across effect paths and reduces mixed-DPI boundary drift.

## Notes
- This is a pragmatic fix to keep effects visually attached to the real cursor.
- If both coordinate sources are wrong (driver issue), further investigation is needed.

## Implementation
- `MFCMouseEffect/MouseFx/Core/GlobalMouseHook.cpp`: `NormalizeScreenPoint(...)`
