# POSIX Core Automation Contract Workflow

## Priority-1 Command
Run this to validate core-lane automation API contracts on macOS:

```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
```

## What It Verifies
1. Core lane is enabled at build time (`-DMFX_ENABLE_POSIX_CORE_RUNTIME=ON`).
2. Test-only probe (`MFX_CORE_WEB_SETTINGS_PROBE_FILE`) can discover runtime WebSettings URL/token.
3. Test-only settings-launch probes verify shell launcher call path without opening a real browser:
   - `MFX_CORE_WEB_SETTINGS_LAUNCH_PROBE_FILE` receives `url/opened`.
   - `MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE` captures launcher command + URL.
4. Core HTTP contracts:
   - settings root URL from probe file returns `200`.
   - `GET /settings-shell.svelte.js?token=<token>` returns `200`.
   - `GET /api/state` requires token header (`401` without header, `200` with header).
   - `POST /api/automation/active-process` returns `200 + ok=true` with non-empty `process` on macOS.
   - `POST /api/automation/app-catalog` returns `200 + ok=true` for both `force=true` and `force=false`.
   - `POST /api/state` can persist selected-app scope from scanned catalog entries (roundtrip verified via `GET /api/state`).
   - `POST /api/automation/test-app-scope-match` (test-gated) verifies process-scope alias matching semantics.
   - `POST /api/automation/test-binding-priority` (test-gated) verifies binding priority semantics (`process > all` on same chain length; `longest chain > shorter chain`).
   - `POST /api/automation/test-shortcut-from-mac-keycode` (test-gated) verifies mac keycode->shortcut mapping semantics.
   - `POST /api/automation/test-inject-shortcut` (test-gated) verifies automation injector call path under deterministic dry-run mode.
   - Startup-missing-permission path is simulated via `MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE=trusted=0` and must converge to degraded `permission_denied`.
   - Startup degraded notification dedup is asserted via `MFX_TEST_NOTIFICATION_CAPTURE_FILE` (single warning before grant).
   - Grant-after-start auto-recovery is asserted by switching simulation file to `trusted=1` without process restart.
   - Runtime revoke/regrant transitions are asserted by toggling the same simulation file (`1 -> 0 -> 1`).
   - `POST /api/input-indicator/test-mouse-labels` (test-gated) verifies mac indicator label pipeline yields `L/R/M`.
   - `POST /api/wasm/catalog` returns `200 + ok=true`.
   - `POST /api/wasm/import-selected` returns `200 + ok=true` when manifest path is valid.
   - `POST /api/wasm/import-from-folder-dialog` with `{"probe_only":true}` returns `200 + ok=true` without modal dialog.
   - `POST /api/wasm/test-dispatch-click` (test-gated) verifies non-interactive click dispatch invoke path.
   - WASM capability fields are present in state (`invoke_supported`, `render_supported`).
5. macOS semantic guard:
   - app catalog must be non-empty.
   - at least one app entry uses `.app` process suffix.
   - selected-app scope roundtrip must preserve a scanned `process:<*.app>` token in state.
   - schema capability must report `capabilities.input.keyboard_injector=true`.
   - shortcut inject probe (`Cmd+C`) must return `accepted=true` when regression runs with dry-run injector mode.
   - `state.wasm.runtime_backend` must be `wasm3_static` (Phase 55a runtime identity guard).
   - `state.wasm.render_supported` must be `true` on current macOS core lane.
   - wasm catalog endpoint must not return `wasm_catalog_not_supported_on_this_platform`.
   - wasm import-dialog probe must return `"supported":true`.
   - wasm test-dispatch response must include `route_active=true`, `invoke_ok=true`, and `rendered_any=true`.
   - app-scope matching must treat `code` / `code.exe` / `code.app` as equivalent aliases.
   - binding priority must keep `process` scope above `all` under same trigger chain length, while preserving longest-chain-first as top rule.
   - shortcut mapping must keep `Cmd+V` and `Cmd+Tab` distinct (`mac_key_code 9 -> Win+V`, `48 -> Win+Tab`).

## Core Options
```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh \
  --platform auto \
  --build-dir /tmp/mfx-platform-macos-core-automation-build
```

## Test-Fast Tuning
```bash
MFX_CORE_HTTP_START_WAIT_SECONDS=1 \
MFX_CORE_HTTP_PROBE_TIMEOUT_SECONDS=8 \
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
```

Test-endpoint gate used by regression startup:
- `MFX_ENABLE_WASM_TEST_DISPATCH_API=1` (default enabled by script; override to `0` when you need to disable the test endpoint)
- `MFX_ENABLE_AUTOMATION_SCOPE_TEST_API=1` (default enabled by script; override to `0` when you need to disable the app-scope test endpoint)
- `MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API=1` (default enabled by script; override to `0` when you need to disable the shortcut-map test endpoint)
- `MFX_ENABLE_AUTOMATION_INJECTION_TEST_API=1` (default enabled by script; override to `0` when you need to disable the injection probe endpoint)
- `MFX_ENABLE_INPUT_INDICATOR_TEST_API=1` (default enabled by script; override to `0` when you need to disable indicator-label probe endpoint)
- `MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN=1` (default enabled by script; makes mac injector return deterministic probe success after parse/map validation)

Settings-launch probe files used by regression startup:
- `MFX_CORE_WEB_SETTINGS_LAUNCH_PROBE_FILE` (script-managed temp file for launch-result probe)
- `MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE` (script-managed temp file for launcher-capture probe)
- `MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE` (script-managed temp file for mac permission simulation)
- `MFX_TEST_NOTIFICATION_CAPTURE_FILE` (script-managed temp file for shell warning capture/dedup assertion)
- `MFX_CORE_HTTP_INPUT_CAPTURE_TIMEOUT_SECONDS` (permission transition wait timeout, default `10`)

## Packaging Layout (Keep This Boundary)
- Orchestrator:
  - `tools/platform/regression/run-posix-core-automation-contract-regression.sh`
- Shared modules:
  - `tools/platform/regression/lib/core_http.sh`
  - `tools/platform/regression/lib/build.sh`
  - `tools/platform/regression/lib/common.sh`
