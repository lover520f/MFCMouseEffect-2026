# Agent Current Context (P1, 2026-03-12)

## Purpose
This file is the compact P1 runtime truth for daily execution.  
Deep implementation details are intentionally moved to P2 docs to reduce context waste.

## Scope and Platform Priority
- Primary host: macOS.
- Delivery priority: macOS mainline first, Windows regression-free, Linux compile/contract-level.
- macOS stack rule:
  - New capability modules are Swift-first.
  - Existing `.mm` surface is maintenance/refactor only; avoid expanding `.mm` scope.

## Runtime Lanes
- Stable lane: `scaffold`
- Progressive lane: `core` (`mfx_entry_posix_host`)
- Policy: new cross-platform capability lands in `core` first, while `scaffold` stays stable.

## Active Product Goals
- Goal A: wasm runtime remains bounded-but-expressive (not raw shader ownership), while improving parity and testability.
- Goal B: input indicator and effect plugins coexist safely by lane/surface separation.
- Goal C: automation gesture mapping remains accurate, observable, and low-regression across macOS/Windows.

## Current Capability Snapshot

### Visual Effects / WASM
- `click/trail/scroll/hold/hover` are active in `core`.
- Shared command tail (`blend_mode/sort_key/group_id`) is active.
- Group retained model is active (transform/material/pass remain host-owned bounded surfaces).
- Compatibility boundary remains:
  - wasm can express rich 2D composition.
  - wasm does not own raw GPU pipeline/shader graph.

### Input Indicator
- macOS and Windows indicator labels/streak semantics are aligned (`L xN`, `W+ xN`).
- Indicator wasm dispatch uses dedicated indicator lanes and safer budget floor.
- Indicator plugin routing exposes explicit route snapshot for diagnostics.
- `SetInputIndicatorConfig` now syncs indicator wasm host immediately after apply (budget + enabled + manifest load/unload), so persisted `wasm_manifest_path` cannot drift from runtime active plugin.
- WebUI `Apply` now always re-renders from backend-confirmed state; when configured indicator manifest differs from runtime active manifest, UI performs one runtime reconcile (`/api/wasm/load-manifest`) then refreshes state.
- Input-indicator plugin menu no longer uses a separate "Load Selected" button:
  - selecting a manifest attempts immediate load (failure rolls back to previous manifest),
  - plugin override control is rendered as a switch and becomes the primary enable/disable action.
- WebUI settings bootstrap is now split into `app-core.js` + `app-actions.js` + `app-gesture-debug.js` (with `app.js` as bootstrap) to lower coupling and shrink the largest runtime file; webui asset discovery prefers directories that contain these files.
- Input-indicator settings logic is now extracted to `settings-form-input-indicator.js` to keep `settings-form.js` focused and reduce coupling between settings sections.
- Input-indicator plugin menu now delegates catalog/route helper logic to `input-indicator-plugin-menu-model.js` to keep the Svelte view focused on UI structure.
- Automation mapping UI now extracts selection/summary/helper logic into `mapping-panel-helpers.js` to shrink `MappingPanel.svelte` and reduce coupling.
- Automation mapping scope UI now lives in `MappingScopePanel.svelte`, keeping `MappingPanel.svelte` focused on orchestration while preserving existing behavior.
- Automation mapping shortcut editor UI is now extracted to `MappingShortcutPanel.svelte` to keep `MappingPanel.svelte` thinner and reduce coupling.
- `/api/wasm/load-manifest` now infers `surface` from the manifest when the caller omits it, preventing indicator-only plugins from being loaded into effects hosts (and vice versa).
- Core/scaffold webui path resolvers now both prefer source-tree/working-dir assets before executable-side bundle in dev runs, and prioritize candidates with complete indicator UI assets (`input-indicator-settings.svelte.js`) to reduce stale frontend bundle pickup.
- Manual macOS core websettings runner now defaults to rebuilding `WebUIWorkspace` before starting host, and pins both `MFX_WEBUI_DIR` + `MFX_SCAFFOLD_WEBUI_DIR` to repo `WebUI`, reducing stale bundle drift between `/tmp` host binaries and source-tree frontend assets.

### Automation Mapping
- App-scope normalization/parser contracts are stable; preset/custom mapping with similarity threshold remains.
- Custom gesture editor uses explicit `Draw -> Save`; trigger button supports `none`.
- Matcher robustness, windowed engine, and ambiguity rejection are active; calibration overrides are available for tests.
- Debug observability keeps `recent_events` plus recognized vs matched split for UI clarity.
- Detailed behavior notes: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/automation-mapping-notes.md`

## Debug and Observability Contract
- Runtime gesture-route diagnostics are gated by debug mode:
  - `./mfx start --debug`
  - or `MFX_RUNTIME_DEBUG=1`
- Default non-debug runs do not emit this lane to avoid overhead.
- WebUI debug rendering is decoupled and mounted in sidebar debug card; runtime state is synced via workspace runtime channel.
- Debug polling is adaptive (active ~66ms, idle ~180ms) and only runs when diagnostics payload exists, connection is online, and the automation section is focused.
- Preview rendering prefers sampled trajectory + explicit recognized/matched fields; detailed UI behavior and fallback logic moved to:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/gesture-debug-ui-notes.md`

## Server Structure Note
- Server layout follows SRP with `core/`, `routes/*`, `settings/`, `diagnostics/`, `http/`, `webui/` sub-layers; include boundaries are tightened.
- Detailed map and include notes: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/server-structure.md`

## Regression Gates (High Frequency)
- Canonical entry: `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`.
- Full workflow and per-phase commands: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`

## High-Value Manual Commands
- Core run/debug and manual test commands are cataloged here:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/ops/manual-commands.md`

## Contracts That Must Not Drift
- Keep stdin JSON command compatibility.
- Keep wasm ABI backward compatibility inside current major ABI contract unless explicit migration approved.
- Keep host-owned bounded pass/material strategy; do not add raw shader controls without architecture approval.
- Keep docs synchronized in the same change set when behavior/contracts change.

## P2 Detail Routing
Read these only when task keywords match:
- P2 index:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
- Automation matching and thresholds:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/gesture-matching.md`
- WASM route and ABI:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.md`
- Workflow contracts:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`

## Documentation Governance State
- `current.md` is P1 only (compact execution truth).
- P2 details are indexed and routed by:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/docs/ai-context.sh route --task "<keywords>"`
- Context artifacts:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-index.json`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-map.md`
- Realtime index refresh options:
  - `./tools/docs/ai-context.sh watch`
  - `./tools/docs/install-git-hook.sh`
