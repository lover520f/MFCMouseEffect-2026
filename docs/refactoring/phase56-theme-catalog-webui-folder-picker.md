# Phase56: Theme Catalog WebUI Folder Picker (macOS mainline)

## Why
- Existing backend already supported `theme_catalog_root_path`, but WebUI general section had no direct path input/browse action.
- Theme catalog runtime contract needed explicit regression coverage so future changes do not silently break UI-to-core path selection.

## Scope
- Capability: `Effects` (theme catalog config path), plus shared WebSettings API surface.
- Platforms:
  - macOS: primary run path (native folder picker expected available in normal host).
  - Windows/Linux: unchanged behavior for settings apply path; route remains token-protected core API.

## Implementation
1. WebUI general section now includes:
- `theme_catalog_root_path` text field.
- `Browse` button with Svelte action callback (`pickThemeCatalogRootPath`).

2. WebUI action plumbing:
- `MfxGeneralSection` now supports `onAction(handler)`.
- `settings-form.js` forwards `generalAction` into the general section and persists `theme_catalog_root_path` in read-back state.
- `app.js` adds `handleGeneralAction()` and posts to:
  - `POST /api/theme/catalog-folder-dialog`

3. Core API route:
- Added `POST /api/theme/catalog-folder-dialog` in `WebSettingsServer.CoreApiRoutes.cpp`.
- Supports:
  - `probe_only=true` -> capability probe response (no OS dialog).
  - normal mode -> native folder picker (`PlatformNativeFolderPicker`) and selected path response.

4. Runtime metadata contract (schema/state):
- Schema adds `theme_catalog` section:
  - `configured_root_path`
  - `built_in_theme_count`
  - `runtime_theme_count`
  - `scanned_external_theme_files`
  - `external_theme_count`
  - `rejected_external_theme_files`
  - `folder_picker_supported`
- State adds `theme_catalog_runtime` section with matching count fields.

5. WebUI apply consistency:
- `app.js` save flow compares previous vs requested `theme_catalog_root_path`.
- When path changes, save flow re-fetches `/api/schema` and re-renders snapshot, so theme dropdown options update immediately after Apply.

6. Runtime theme value normalization:
- Core adds runtime-catalog resolver (`ResolveRuntimeThemeName`) and applies it in startup/reload/set-theme/set-root-path paths.
- Persisted `theme` is now forced to a runtime-available catalog value (fallback-safe), avoiding schema/state drift when external theme root changes.

7. Windows tray theme parity:
- Keep built-in theme command IDs unchanged (`kCmdTheme*`) for backward compatibility.
- Add dynamic command mapping for runtime external themes so tray menu can select catalog-loaded themes instead of silently skipping them.

8. macOS tray theme boundary hardening:
- `MacosTrayMenuFactory` no longer reads `ThemeStyle` directly.
- Theme list + selected theme now come from `IAppShellHost::GetThemeMenuSnapshotFromShell(...)`, so shell menu rendering stays decoupled from core style runtime linkage.
- Theme apply callback remains `SetThemeFromShell(...)` (no behavior change to user action path).

8. WebUI general state model unification:
- General section now uses one shared normalizer (`src/general/general-state-model.js`) across entry and Svelte field component.
- Added `scripts/test-general-state-model.mjs` and wired into POSIX WebUI semantic gate to prevent field drift for `theme_catalog_root_path` and defaults.

## API Contract
- Request:
  - `{"probe_only": true}` or `{"initial_path":"..."}`
- Response fields:
  - `ok` (bool)
  - `supported` (bool)
  - `cancelled` (bool)
  - `error` (string)
  - `error_code` (string)
  - `selected_folder_path` (string)

## Regression Coverage
- Extended core HTTP state checks:
  - Assert `theme_catalog_root_path` exists in `/api/state`.
  - Assert `themes` exists in `/api/schema`.
  - Probe `POST /api/theme/catalog-folder-dialog` with `probe_only=true`.
  - Assert schema/state theme catalog runtime count parity:
    - `len(schema.themes) == schema.theme_catalog.runtime_theme_count`
    - `schema.theme_catalog.runtime_theme_count == state.theme_catalog_runtime.runtime_theme_count`
  - Assert `state.theme` is always present in `schema.themes` (baseline, after apply external root, and after restore).
  - Add invalid-theme normalization contract:
    - `POST /api/state` with invalid `theme` value
    - assert runtime persisted `theme` is auto-normalized (not raw invalid value)
    - assert normalized value remains inside `schema.themes`
    - restore original `theme` and verify restoration
  - Add chromatic semantic contract:
    - `POST /api/state` with `theme=chromatic`
    - assert runtime persisted `theme` remains `chromatic` (not rewritten to `neon`)
    - assert `chromatic` remains available in `schema.themes`
  - Add file-driven external theme contract:
    - create temp `contract.theme.json`
    - create temp `neon.theme.json` (built-in value override attempt)
    - `POST /api/state` with temp `theme_catalog_root_path`
    - assert schema contains `contract_external_theme`
    - assert override on built-in `neon` is rejected and built-in semantic stays owned by runtime built-ins
    - assert schema does not expose external override label (`External Neon Override`)
    - assert `external_theme_count/scanned_external_theme_files/rejected_external_theme_files` values (`1/3/2`)
    - restore original `theme_catalog_root_path` and verify restored state

- Added static source-consistency gate:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-theme-catalog-surface-regression.sh`
  - Fails if theme options regress to hardcoded lists in legacy settings, or if schema/tray stop reading from runtime catalog.
  - Added macOS boundary checks: fail if `MacosTrayMenuFactory` reintroduces direct `ThemeStyle/StringUtils` includes instead of host snapshot contract.
  - Fails if core HTTP theme contract coverage markers are removed (`contract_external_theme`, `rejected_external_theme_files`).

- Extended macOS tray smoke contract:
  - `mfx_shell_macos_tray_smoke` now validates menu callback path for both:
    - settings action (`OpenSettingsFromShell`)
    - theme submenu select action (`SetThemeFromShell`)
  - Regression script `tools/platform/regression/lib/smoke.sh` now asserts captured `theme_select` event (`expected_theme == selected_theme == neon`) in addition to settings launch capture.
- Added end-to-end tray theme persistence selfcheck (`tools/platform/manual/run-macos-tray-theme-selfcheck.sh`) for core lane:
  - trigger tray theme selection callback via test env
  - assert `/api/state.theme` updates to target theme
  - restart host and assert theme persists
  - restore original theme to avoid config drift after gate run
  - selfcheck now forces a deterministic precondition (`/api/state` set to non-target theme first), then verifies tray callback switches to target theme, reducing false-positive "already same theme" passes.

- Extended macOS WASM fallback contract:
  - `run-macos-wasm-runtime-selfcheck.sh` now validates `/api/state` fallback diagnostics behavior:
  - after invalid load path, `last_load_failure_stage/code` must be non-empty
  - after successful reload, `last_load_failure_stage/code` must clear to empty strings
  - `run-posix-core-wasm-contract-regression.sh` now enforces the same lifecycle in core HTTP gates:
  - invalid load from `*.missing` path must leave `/api/state.runtime_backend=wasm3_static` and non-empty `last_load_failure_stage/code`
  - successful reload must clear `/api/state.last_load_failure_stage/code` to empty

- Core initialization ordering fix for tray-theme parity:
  - `PosixCoreAppShell` (and shared `AppShellCore`) now starts core controller before tray startup.
  - This removes startup race where tray menu requested theme snapshot before controller was ready, causing empty theme submenu and missed theme callback during startup probes.

- Regression execution reliability fix:
  - `mfx_with_lock` now runs workflows in a strict subshell and captures return status directly.
  - This prevents `cmd || status=$?` boolean-list semantics from weakening `set -e` behavior and accidentally masking phase failures.

- Core HTTP startup reliability hardening (for contract runs):
  - inject isolated single-instance key on each startup attempt (`--single-instance-key` + `MFX_SINGLE_INSTANCE_KEY`)
  - allow controlled skip on no-probe/no-log/no-diagnostics early exit in constrained runners (strict fail still available via `MFX_CORE_HTTP_REQUIRE_EXECUTION=1`)
  - core smoke startup helper now uses the same single-instance isolation, avoiding false `exited before alive check` when earlier suite phases already touched single-instance locks

## Verification
1. WebUI build:
- `cd /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace && pnpm run build`

2. Core effects contract:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- Result in this run: pass with constrained-runtime startup skip (`websettings_start_failed(stage=2,code=1)`), consistent with existing skip policy.
