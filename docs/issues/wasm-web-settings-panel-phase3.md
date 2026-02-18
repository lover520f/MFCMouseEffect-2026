# WASM Web Settings Panel (Phase 3 Increment)

## Summary

This change lands the first visible Phase 3 management surface for WASM plugins in the existing Web settings page.

Goals of this increment:
- keep the current C++ host architecture unchanged,
- expose plugin catalog and lifecycle controls in settings,
- keep diagnostics readable without adding heavy coupling.

## Scope

### Backend (`WebSettingsServer`)
- Added endpoint: `POST /api/wasm/catalog`
- Uses `WasmPluginCatalog::Discover()` to return:
  - `plugins[]`: `id`, `name`, `version`, `api_version`, `manifest_path`, `wasm_path`
  - `errors[]`
  - `count`, `error_count`

### Frontend (WebUI + Svelte workspace)
- New Svelte entry: `src/entries/wasm-main.js`
- New section component: `src/wasm/WasmPluginFields.svelte`
- Wired into shell sections as new card: `section_wasm_plugin`
- Added action bridge in `WebUI/app.js`:
  - `catalog`
  - `enable`
  - `disable`
  - `reload`
  - `loadManifest`
- Added i18n keys (EN/ZH) for labels, statuses, and action messages.

### Build/Packaging
- Added `build:wasm` to workspace scripts and `vite` mode target.
- Added output copy entry: `wasm-settings.svelte.js`.
- Added `<script src="/wasm-settings.svelte.js"></script>` in `WebUI/index.html`.
- Added Inno Setup preflight check for `webui/wasm-settings.svelte.js`.

## Design Notes

- WASM panel is isolated as its own section component, matching single-responsibility and low coupling requirements.
- Existing automation/effects/text sections are not touched functionally.
- The panel is read-heavy and command-light:
  - state render from `payload.state.wasm`,
  - lifecycle actions routed through existing API pattern.

## Verification

1. `pnpm run build` in `MFCMouseEffect/WebUIWorkspace`
- Success, includes `dist/wasm-settings.svelte.js`.

2. Release build
- `MSBuild` (`Release|x64`) succeeds after clearing stale process/file lock.
- Confirms backend route compile and post-build WebUI copy path.

## Known Constraints

- This increment does not yet add advanced plugin profile editing.
- Current catalog UI uses single selection + load action (sufficient for current host lifecycle model).
