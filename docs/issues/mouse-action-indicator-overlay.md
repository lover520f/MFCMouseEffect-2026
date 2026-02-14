# Mouse Action Indicator Overlay

## Background
A new visual helper is required to show mouse actions near the cursor without affecting normal input behavior.

## What Was Added
- New runtime component: `MouseActionIndicator`
  - Files:
    - `MFCMouseEffect/MouseFx/Core/MouseActionIndicator.h`
    - `MFCMouseEffect/MouseFx/Core/MouseActionIndicator.cpp`
- Event coverage:
  - Left click: single / double / triple
  - Right click: single / double / triple
  - Wheel: up / down
- Render model:
  - Layered, click-through, no-activate popup window
  - GDI+ ARGB drawing and timer-driven animation

## Positioning Modes
- Relative mode:
  - Anchor to cursor with `offset_x / offset_y`
- Absolute mode:
  - Use virtual desktop coordinates `absolute_x / absolute_y`
  - Works for multi-monitor because coordinates are interpreted in global screen space

## Configuration and Persistence
- Added to `EffectConfig`:
  - `mouse_indicator.enabled`
  - `mouse_indicator.position_mode`
  - `mouse_indicator.offset_x`, `offset_y`
  - `mouse_indicator.absolute_x`, `absolute_y`
  - `mouse_indicator.size_px`
  - `mouse_indicator.duration_ms`
- Load/save path:
  - `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`
- Default strategy:
  - For legacy configs without `mouse_indicator`, default behavior is `enabled=true` so the feature is visible out of the box.

## Runtime Wiring
- `AppController` updates and dispatches indicator events from:
  - `WM_MFX_CLICK`
  - `WM_MFX_SCROLL`
- VM suppression integration:
  - indicator is hidden when VM foreground suppression is active

## Black Square Fix
- Symptom:
  - indicator window appears as a black square with no visible animation.
- Root causes:
  - mixed use of `SetLayeredWindowAttributes` and `UpdateLayeredWindow`.
  - unreliable alpha path when using `GDI+ GetHBITMAP`.
- Fix:
  - remove `SetLayeredWindowAttributes`.
  - switch to a single render path: `CreateDIBSection(32-bit ARGB) + GDI+ draw + UpdateLayeredWindow`.
  - add debug logging for `UpdateLayeredWindow` failures with `GetLastError()`.

## Incremental Fixes: Wheel Detection / Keyboard Support / Visual Overflow
- Wheel and middle-click detection:
  - removed the "unknown button -> left button" fallback in `MouseActionIndicator::OnClick`, so misclassified events no longer render as `L`.
  - extended low-level mouse mapping for button double-click messages (`WM_*BUTTONDBLCLK`) and unified wheel-delta handling (`delta=0` ignored).
- Keyboard support:
  - added `WH_KEYBOARD_LL` in `GlobalMouseHook`, posting `WM_MFX_KEY` on `WM_KEYDOWN/WM_SYSKEYDOWN`.
  - wired `WM_MFX_KEY` handling in `AppController` to `MouseActionIndicator::OnKey`.
  - added `mouse_indicator.keyboard_enabled` in Web settings with state/apply/persist support.
- Visual quality and bounds:
  - button highlight now renders inside a rounded-rect clip path, removing rectangular overflow artifacts.
  - placement remains clamped to virtual desktop bounds to avoid off-screen windows.
  - compact labels (`L2/R2/M2/W+`) reduce text crowding on small indicator sizes.

## Web Settings Integration
- Backend schema/state:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Frontend controls:
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUI/app.js`
- UI options include:
  - enable switch
  - position mode
  - relative/absolute coordinates
  - size and animation duration

## Architecture Refactoring: IndicatorRenderer Extraction
- All GDI+ rendering extracted from `MouseActionIndicator` into:
  - `MFCMouseEffect/MouseFx/Renderers/Indicator/IndicatorRenderer.h`
  - `MFCMouseEffect/MouseFx/Renderers/Indicator/IndicatorRenderer.cpp`
- `MouseActionIndicator.cpp` now:
  - Uses RAII `GdiRenderContext` to prevent GDI resource leaks.
  - Delegates all drawing to `IndicatorRenderer::RenderMouseAction()` / `RenderKeyAction()`.
  - Window class registration uses a bool guard to avoid repeated calls.
- Visual improvements in `IndicatorRenderer`:
  - Click ripple: expanding ring effect on left/right/middle click.
  - Wheel arrow: larger arrows with trailing ghost for visual feedback.

## Keyboard Combo Key Support
- `KeyEvent` now includes modifier state (`ctrl`, `shift`, `alt`, `win`).
- `GlobalMouseHook::KeyboardHookProc` detects modifiers via `GetAsyncKeyState`.
- Display logic:
  - Combo keys: `Ctrl+C`, `Alt+Tab`, `Ctrl+Shift+S`.
  - Modifier-only press: shows just the modifier name (e.g. `Ctrl`).

## Scroll Streak
- Continuous scrolling within 500ms accumulates a count (e.g. `W+ 3`, `W- 5`).
- Resets on direction change or timeout.

## Verification Checklist
1. Enable indicator in Web settings and press Apply.
2. Verify left/right single-double-triple animations with click ripple.
3. Verify wheel up/down animations with enlarged arrows.
4. Switch between relative and absolute mode; confirm position behavior.
5. In multi-monitor setup, verify absolute coordinates are mapped in virtual desktop space.
6. Enter VM foreground window; confirm indicator is suppressed together with other effects.
7. Toggle "Enable keyboard indicator" in Web settings; verify key-press visualization follows the switch.
8. Press `Ctrl+C`, verify combo key display as "Ctrl+C".
9. Press `Ctrl` alone, verify it displays "Ctrl" (not "Ctrl+").
