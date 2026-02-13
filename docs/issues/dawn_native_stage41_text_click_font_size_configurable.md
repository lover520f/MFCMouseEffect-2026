# Stage 41 - Text Click Font Size Configurable via Web Settings

## Problem
- Click text effect could not adjust font size from settings UI.
- `TextConfig::fontSize` existed in config model, but Web settings only exposed text content.

## Root cause
- Missing field propagation on the Web settings route:
  - no input control in Web UI
  - no state value returned by Web settings server
  - no apply handling in `AppController::HandleCommand("apply_settings")`

## Fix
1. Added a new Web UI input:
   - `text_font_size` (number, `6..96`, step `0.5`)
2. Returned `text_font_size` in `/api/state`.
3. Added apply handling in `AppController`:
   - `SetTextEffectFontSize(float sizePt)`
   - clamp to `6.0..96.0`
   - persist config
   - if click effect is currently `text`, recreate it for immediate effect
4. Added i18n labels:
   - zh: `文字大小（pt）`
   - en: `Text font size (pt)`

## Files changed
- `MFCMouseEffect/WebUI/index.html`
- `MFCMouseEffect/WebUI/app.js`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `MFCMouseEffect/MouseFx/Core/AppController.h`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`

## Validation
1. Open Web settings.
2. Set click effect to `text`.
3. Change `Text font size (pt)` and click Apply.
4. Click mouse and confirm floating text size changes immediately.
