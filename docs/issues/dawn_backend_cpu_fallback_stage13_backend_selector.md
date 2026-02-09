# Dawn Backend + CPU Fallback (Stage 13 Backend Selector)

## Goal
- Expose render backend selection directly in Web settings UI so users can switch CPU/Dawn intentionally.
- Keep this aligned with existing backend awareness banner from Stage 12.

## Changes

### 1) General Settings: Render Backend Selector
- Updated:
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUI/app.js`
- Added field:
  - `render_backend` selector in the General card.
- Data source:
  - `schema.render_backends` (already provided by backend).
- State binding:
  - `reload()` now binds selector value from `state.render_backend`.
  - `buildState()` now posts `render_backend` in `/api/state`.

### 2) i18n Labels
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Added keys:
  - `label_render_backend` (en/zh).

## Result
- Users can now choose desired backend in settings (CPU or Dawn where available).
- Combined with Stage 12 banner + probe button, users can both control and verify runtime backend behavior.
