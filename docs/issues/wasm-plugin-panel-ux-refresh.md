# WASM Plugin Panel UX Refresh (2026-02)

## Background
- The previous panel flattened policy, catalog, controls, and diagnostics into one long grid.
- Key status signals (loaded/not loaded, active plugin, runtime backend) had weak first-screen hierarchy.
- `Manifest path` and `Configured manifest path` were often duplicated and looked confusing.

## Changes
1. Information architecture
- Added a top summary strip: `plugin loaded / runtime backend / active plugin / last call metrics`.
- Split the panel into 3 blocks:
  - `Plugin Catalog`: scan, select, load, catalog errors, and catalog roots.
  - `Runtime Policy`: enable/disable, fallback policy, budget knobs, and effective runtime budget.
  - `Diagnostics`: render stats, budget flags, budget reason, parse/host/render errors.

2. Path clarity
- Main path display now uses a single `Manifest path` row (runtime first, fallback to configured).
- If runtime and configured paths are the same, show one row with a synced badge.
- Show `Configured manifest path` only when it differs.

3. Visual hierarchy
- Added block containers, section headers, helper descriptions, and summary cards.
- Added inline status badge for manifest sync.
- Kept the existing theme language and color system.

4. Runtime policy first-row tuning
- Moved runtime control buttons (`Enable/Disable/Reload`) to the first row in the Runtime Policy block.
- Renamed the label from `Runtime controls` to `Runtime actions` to better match immediate command semantics.

## Files
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- `MFCMouseEffect/WebUI/styles.css`
- `MFCMouseEffect/WebUI/i18n.js`

## Regression checklist
- New i18n keys render correctly in both English and Chinese.
- Summary strip collapses cleanly on narrow screens.
- `Configured manifest path` visibility is correct for both synced and divergent states.

## Follow-up Fix (dynamic i18n row)
- Symptom: under Chinese locale, the late-rendered operation row still showed `Operation result`.
- Cause: the row is conditionally inserted and relied on static fallback text.
- Fix: render this label via `text('label_wasm_operation_result', ...)` directly in component logic.
