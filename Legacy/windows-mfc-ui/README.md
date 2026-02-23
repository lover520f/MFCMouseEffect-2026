# Legacy Windows MFC UI Archive

This directory stores historical MFC UI source files that are no longer part of
the active build pipeline.

Current executable wiring uses the platform shell architecture under:

- `MFCMouseEffect/Platform/windows/...`
- `MFCMouseEffect/MouseFx/Core/...`

Why this archive exists:

1. Keep migration history and old implementation details for reference.
2. Keep active source tree focused on current platformized runtime.
3. Avoid accidental re-coupling with legacy MFC Doc/View components.
