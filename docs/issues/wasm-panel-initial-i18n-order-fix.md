# WASM Panel Initial i18n Order Fix

## Problem
- In Chinese mode, some WASM panel labels showed English on first load.
- Clicking `Reload` made those labels switch to Chinese.

## Root Cause
- `applyI18n(uiLang)` was executed before section components finished mounting.
- Newly mounted Svelte DOM nodes with `data-i18n` were not present during the first i18n pass.
- There was also an init race: if `MfxWasmSection` registered after the first `settings-form.render`, WASM section render was skipped until manual reload.
- There is another lazy-mount path: section DOM can be inserted after the first i18n pass (for example after workspace section switching), leaving fallback English until the next global reload.

## Fix
- Keep the existing pre-render `applyI18n(uiLang)` call.
- Add a second `applyI18n(uiLang)` immediately after section render/mount.
- This guarantees newly inserted WASM section nodes are translated in the first load cycle.
- Add deferred retry in `settings-form.js`: when WASM section bridge is not ready, cache the render payload and retry shortly, so first load state/i18n still reaches WASM section without manual reload.
- Add a lightweight `MutationObserver` in `i18n-runtime.js`: when new nodes (or i18n attributes) are inserted, current language is applied automatically to `data-i18n` / `data-i18n-title` / `data-i18n-placeholder` nodes. This removes the manual-reload dependency at the root.

## File
- `MFCMouseEffect/WebUI/app.js`
- `MFCMouseEffect/WebUI/settings-form.js`
- `MFCMouseEffect/WebUI/i18n-runtime.js`
