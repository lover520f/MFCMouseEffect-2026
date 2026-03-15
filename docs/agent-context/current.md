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
- Input-indicator basic settings apply regression fixed: `form` state is now two-way bound across `InputIndicatorFields -> InputIndicatorSectionTabs -> InputIndicatorBasicFields`, so edits in the basic tab (`position_mode`, offsets, size/duration, etc.) always flow back to the save payload instead of snapping back to previous backend state after `Apply`.
- Input-indicator basic settings layout fix: the per-monitor overrides header/container now span both grid columns, so when overrides are hidden the following labels no longer shift into the right column (prevents label/input misalignment).
- Input-indicator plugin menu fixed a selection lock issue: the selector no longer overwrites user-selected manifest with stale prop value before `loadManifest` executes, so switching between indicator plugins can take effect.
- WebUI wasm action refresh now keeps UI edits while always trusting backend `wasm_manifest_path`, preventing indicator plugin selection from reverting after refresh.
- WebUI now exposes a unified top-level `Plugin Management` section for shared plugin lifecycle actions:
  - centralized `catalog root`, `import`, `export`, and cross-surface catalog browse (`effects`/`indicator`),
  - effects/input-indicator secondary tabs keep runtime selection + per-surface configuration, while import/export entry is consolidated.
- Effects plugin secondary tab UX trimmed:
  - removed local `Refresh Plugin List` action from effect plugin binding panel,
  - runtime plugin detail rows (`plugin api version`, paths, fallback reason) are now collapsed by default behind an expandable block.
  - runtime diagnostics panel is now collapsed by default in effects section to reduce visual noise on first open.
  - operation results now rely on the shared top status bar (no extra in-panel status block).
  - runtime policy now uses a collapsible details block; global runtime actions (`Using Plugin` switch + `Reload Plugin`) are pinned above the fold and remain visible even when policy details are collapsed.
  - runtime actions row now uses compact left-aligned layout without the extra inline label, removing the wide two-column gap in the runtime policy header area.
  - runtime actions (`Using Plugin` switch + `Reload Plugin`) are now placed in the runtime-policy header top row (right side on desktop, stacked on narrow screens) to reduce vertical whitespace.
  - runtime policy block is now ordered before plugin-binding details in the effects plugin tab, reducing vertical jump for frequent enable/disable operations.
  - per-channel plugin controls are switch toggles: `Enable` to bind selected plugin, click again to clear channel binding (`Loaded` state reflects current selected binding).
  - channel dropdown now only changes the candidate manifest; it no longer writes policy immediately. Effective binding changes only when the channel switch is toggled.
  - per-channel toggle state updates immediately after action success and reflects channel-specific binding (not global manifest fallback), while still reconciling with backend state on next snapshot.
  - channel toggle now uses optimistic local override (immediate on/off UI feedback) and automatically rolls back on action failure to prevent stale-looking switch state.
  - fixed a Svelte reactivity pitfall in per-channel toggle status: template expressions now depend on explicit per-channel configured-binding state (instead of hidden closure reads from `current`), so switch status refreshes immediately without requiring manual `Cmd+R`.
- Plugin management section visual cleanup:
  - removed duplicated in-panel `Plugin Management` title/description to avoid repeated heading block with workspace context/card header,
  - promoted `Catalog root path` to explicit field label for clearer scan-path editing.
  - catalog list now uses a fixed-height scroll area with manifest-path ellipsis (full path in hover title) to prevent long plugin lists from overwhelming the page.
  - scan-root hint no longer repeats when the discovered root is identical to configured root.
  - controls now use two aligned toolbars (scan path / surface + actions), and each plugin row is rendered as a compact card with surface pill for clearer visual hierarchy.
  - plugin manager list styles are now anchored in global `/WebUI/styles.css` (instead of component-local CSS only), preventing plain-text stacked rendering when workspace runtime does not load mode-extracted component CSS.
  - plugin row labels refined (`适用范围` / `插件路径`) for better readability; removed bottom in-panel "operation completed" status block to reduce visual noise.
- Official runtime indicator sample set is now reduced to `demo.indicator.basic.v2` only; non-basic indicator demos and synced runtime artifacts are removed to prevent stale plugin selection drift.
- `SetInputIndicatorConfig` now auto-recovers stale indicator wasm manifest paths: when `render_mode=wasm` but configured manifest no longer exists, controller degrades indicator render mode to `native` and clears `wasm_manifest_path`, preventing repeated "manifest file does not exist" apply failures.
- Automation mapping UI now extracts selection/summary/helper logic into `mapping-panel-helpers.js` to shrink `MappingPanel.svelte` and reduce coupling.
- Automation mapping scope UI now lives in `MappingScopePanel.svelte`, keeping `MappingPanel.svelte` focused on orchestration while preserving existing behavior.
- Automation mapping shortcut editor UI is now extracted to `MappingShortcutPanel.svelte` to keep `MappingPanel.svelte` thinner and reduce coupling.
- `/api/wasm/load-manifest` now infers `surface` from the manifest when the caller omits it, preventing indicator-only plugins from being loaded into effects hosts (and vice versa).
- `/api/wasm/load-manifest` now also promotes indicator render mode to `wasm` after successful indicator manifest switch, so plugin selection takes effect immediately and survives refresh.
- Core wasm HTTP contract checks assert `indicator-basic` load consistency (configured/active manifest and `input_indicator.render_mode`) after indicator manifest apply.
- Core/scaffold webui path resolvers now both prefer source-tree/working-dir assets before executable-side bundle in dev runs, and prioritize candidates with complete indicator UI assets (`input-indicator-settings.svelte.js`) to reduce stale frontend bundle pickup.
- Manual macOS core websettings runner now defaults to rebuilding `WebUIWorkspace` before starting host, and pins both `MFX_WEBUI_DIR` + `MFX_SCAFFOLD_WEBUI_DIR` to repo `WebUI`, reducing stale bundle drift between `/tmp` host binaries and source-tree frontend assets.
- Workspace sidebar section order is now: `General -> Cursor Effects -> Input Indicator -> Automation Mapping -> Plugin Management`; effects section title is renamed from `Visual Effects` to `Cursor Effects` (`光标特效`).
- Cursor effects now include a secondary tab `Effect Blacklist` (`effects_blacklist_apps`): when foreground process matches the blacklist, click/trail/scroll/hold/hover rendering is skipped (native + WASM), while automation and input-indicator lanes remain active.
- `Effect Blacklist` app selection now reuses automation scope catalog UI (`MappingScopePanel` + `/api/automation/app-catalog`): supports search, refresh, and pick-from-file; free-text manual add is disabled to keep scope entries consistent with catalog/file sources.
- `Effect Blacklist` UI now uses the same two-column scope layout as automation mapping, and blacklist chip state updates immediately after selecting catalog items (no manual page refresh needed).
- `Effect Blacklist` card no longer repeats in-panel title/description, and two-column layout collapses only on narrow widths (`<=720px`) to keep desktop-style left/right scope panes in normal settings window sizes.
- `Effect Blacklist` now keeps a local pending selection snapshot between backend polling renders; catalog clicks no longer get cleared before `Apply`, and pending state auto-clears once backend state catches up.
- `Apply` payload now includes `effects_blacklist_apps` from effects section read model (`WebUI/settings-form.js`), fixing post-apply refresh loss where blacklist chips appeared selected but were not persisted.
- `Effect Blacklist` file-picker filter now reuses automation mapping helper (`scopeFileAccept`) so picker behavior stays identical between both modules.
- macOS automation app catalog now scans `/System/Library/CoreServices` in addition to `/Applications` and `/System/Applications`, so core system apps (for example Finder) appear in scope/blacklist app pickers.
- Workspace card bodies no longer repeat section title/description headers (single source of truth is the top workspace context header), while sidebar/context titles still resolve from section i18n metadata.
- Workspace sidebar no longer renders the focused-view helper sentence; left panel keeps only section navigation plus automation debug card when active.
- `General` section now includes a persisted `launch_at_startup` toggle (`开机启动`) in apply/read/state/config JSON flow, so startup preference no longer gets dropped on refresh.
- macOS `launch_at_startup` now has native side effect via LaunchAgent (`~/Library/LaunchAgents/com.mfcmouseeffect.autostart.plist`): enabling writes/updates `ProgramArguments=[current_executable, "--mode=background"]`, disabling removes the plist.
- `launch_at_startup` now applies immediately on macOS: toggle action also runs `launchctl bootout/bootstrap` for current user session (no need to wait for next login).

### Automation Mapping
- App-scope normalization/parser contracts are stable; preset/custom mapping with similarity threshold remains.
- Custom gesture editor uses explicit `Draw -> Save`; trigger button supports `none`.
- Matcher robustness, windowed engine, and ambiguity rejection are active; calibration overrides are available for tests.
- Debug observability keeps `recent_events` plus recognized vs matched split for UI clarity.
- Shortcut capture on macOS now includes `event.code` symbol fallback (`BracketLeft/BracketRight/...`) when `event.key` is transformed by layout/IME, fixing cases like `Cmd+]` being recorded as modifier-only `Cmd`.
- Native shortcut capture/injection path now also supports punctuation keys end-to-end (`BracketLeft/BracketRight/Backslash/...`): macOS keycode mapping, shortcut text formatting, chord parsing, and macOS keyboard injection tables are aligned, so automation mouse mappings and gesture mappings both accept combos like `Cmd+]`.
- Gesture trigger-button option order now follows backend schema order (no frontend re-sort), so newly added mappings keep consistent default selection with runtime policy and no longer appear as "default selected last option".
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
