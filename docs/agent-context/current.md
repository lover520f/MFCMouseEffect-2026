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
- Workspace sidebar section order is now: `General -> Mouse Companion -> Cursor Effects -> Input Indicator -> Automation Mapping -> Plugin Management`; effects section title remains `Cursor Effects` (`光标特效`).
- New cursor-centric 3D companion capability is now exposed as `Mouse Companion` (`鼠标伴宠`) instead of `3D萌宠`:
  - config/state key: `mouse_companion`
  - macOS runtime: Swift-first SceneKit bridge (`Platform/macos/Pet/MacosMouseCompanionBridge.swift`)
  - test profile switch is built in (`use_test_profile`) with dedicated test parameters for fast verification.
- Mouse Companion 3D runtime blueprint P0 is now scaffolded in `MouseFx/Core/Pet` with canonical-format decision locked to `glb` (`glb` as internal runtime format; `gltf/usdz/vrm/fbx` as import inputs), and action pipeline baseline explicitly avoids model built-in frame animation:
  - docs: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-3d-runtime-blueprint.zh-CN.md`
- Mouse Companion P1 input-action bridge is now wired through `PetDispatchFeature` and `AppController`:
  - `DispatchRouter` now forwards `click/move/button-down/button-up` to pet action routing (`ClickReact/Follow/Drag` baseline),
  - P1 historical path used `CreateNullPetModelRuntime()` only as temporary non-rendering safety lane.
- Mouse Companion P2 runtime path now mounts `CreateDefaultPetModelRuntime()` and includes a minimal `glb` skeleton loader (`skins/joints + nodes parent graph` from `glb` JSON chunk):
  - startup tries default model `Assets/Pet3D/source/pet-main.glb` (with repo-prefixed fallback path),
  - if model is absent or parse fails, companion path still degrades safely without affecting existing effects/automation lanes.
  - pose ingestion now uses `boneIndex`-first resolution (with name fallback), reducing per-frame string lookup pressure and aligning with future render-bridge data flow.
  - `IActionSynthesizer` now has explicit `BindSkeleton(const SkeletonDesc*)` contract; `PetCompanionRuntime` binds skeleton after model load.
  - default synthesizer now emits procedural rig poses (`hips/spine/chest/neck/head`) by skeleton slot instead of relying on model built-in frame animation.
- Mouse Companion P3 minimal visual path is active on macOS:
  - Swift/SceneKit bridge C API is wired (`create/show/hide/load/update`),
  - controller now forwards runtime action state (`cursor/action/intensity/boneCount`) to visual host,
  - `mfx_entry_runtime_common` build gate passes with Swift 6 actor-safety constraints.
- Mouse Companion P4 pose bridge is now in indexed mode:
  - `PetCompanionRuntime::LastPose()` is exposed for controller-side bridge encoding,
  - controller sends per-frame pose arrays (`boneIndex + position/rotation/scale`) through `mfx_macos_mouse_companion_apply_pose_v1`,
  - pose binding is cached via `mfx_macos_mouse_companion_configure_pose_binding_v1` (skeleton index -> SceneKit node),
  - Swift host now resets mapped bones to rest local transform before applying each frame, preventing stale pose carryover.
- Mouse Companion P5 first slice (external action clips) is now wired:
  - action asset contract is split into `ActionLibrary` (`MouseFx/Core/Pet/PetActionLibrary.h/.cpp`) with JSON loader,
  - synthesizer now supports `SetActionLibrary(...)` and does clip sampling per action before procedural fallback,
  - startup now attempts default clip library `Assets/Pet3D/source/pet-actions.json` (with repo-prefixed fallback),
  - detailed clip schema/rules: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-action-clip-contract.zh-CN.md`.
- Mouse Companion P6 first slice (appearance customization) is now wired:
  - appearance profile loader added (`PetAppearanceProfile`, default path `pet-appearance.json`),
  - runtime now persists current `AppearanceOverrides` and forwards it to macOS bridge,
  - Swift bridge adds `mfx_macos_mouse_companion_apply_appearance_v1` for accessory visibility + texture override apply/rollback.
- Mouse Companion P7 first slice (import pipeline decoupling) is now wired:
  - importer conversion stage is abstracted to `IModelFormatConverter` (`ModelConversionResult`, `Supports`, `ConvertToCanonicalGlb`),
  - default converter is a composite pipeline (`glb` passthrough + `vrm` real converter + `gltf` real converter + `usdz/fbx` tool-backed real converter + sidecar fallback resolver),
  - VRM converter now validates binary glTF header and exports/reuses `canonical/<stem>.glb` with fallback-to-source behavior on copy/permission failures,
  - GLTF converter now exports JSON-only canonical `.glb` and copies external resources into `canonical/` with cache reuse,
  - USDZ/FBX converters now support command-template backend with env overrides (`MFX_PET_USDZ_TO_GLB_COMMAND`, `MFX_PET_FBX_TO_GLB_COMMAND`),
  - converter diagnostics now include `converter.vrm.*` + `converter.gltf.*` + `converter.usdz.*` + `converter.fbx.*` categories for clearer triage,
  - `CreateModelAssetImporter(std::unique_ptr<IModelFormatConverter>)` allows future real converter backend injection without touching controller/runtime contracts,
  - detailed contract: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-model-import-pipeline-contract.zh-CN.md`.
- Mouse Companion P8 first slice (WebSettings contract wiring) is now wired end-to-end:
  - WebSettings sidebar adds `Mouse Companion` section (`section_mouse_companion`),
  - web apply payload now includes `mouse_companion` and persists via `EffectConfig` (`model_path/size/offset/smoothing/test-profile` fields),
  - settings schema/state now expose `mouse_companion` ranges + current values for UI read/write parity,
  - controller now has `SetMouseCompanionConfig(...)`; runtime dispatch and visual host are gated by `mouse_companion.enabled`,
  - macOS visual host creation now uses configured `mouse_companion.size_px`, and model loading now prefers configured `mouse_companion.model_path` before default fallback candidates.
- Mouse Companion P8 second slice (runtime parameter activation) is now wired:
  - dispatch runtime now consumes active profile (`production` or `test`): `smoothing_percent`, `follow_threshold_px`, `release_hold_ms`,
  - follow routing now supports low-jitter throttling (`follow_threshold_px`) + smoothed cursor input (`smoothing_percent`) before action synth tick,
  - primary button release now honors `release_hold_ms` by keeping a short drag tail before returning to follow action,
  - macOS bridge now accepts follow profile sync (`offset_x/offset_y/press_lift_px`) and applies configured offsets/lift in window follow update.
- Mouse Companion P9 first slice (asset-path runtime control) is now wired:
  - `mouse_companion` config/schema/state/apply now includes `action_library_path` + `appearance_profile_path`,
  - controller compare/apply path changes and performs targeted hot reload (without forcing model reload when unnecessary),
  - action/appearance default resolver now prefers configured path first, then falls back to built-in candidate paths.
- Mouse Companion runtime feasibility proof is validated on 2026-03-17 (macOS core host, local API readback):
  - `mouse_companion_runtime.model_loaded=true`, `action_library_loaded=true`, `appearance_profile_loaded=true`,
  - `mouse_companion_runtime.skeleton_bone_count=30`, `visual_host_active=true`, `model_load_error=""`,
  - in constrained/no-tray sessions, use `--mode=background` with a kept-open stdin pipe for stable host lifetime before calling `/api/state`.
- Mouse Companion P9 second slice (visual runtime observability + pose-binding readiness) is now wired:
  - `mouse_companion_runtime` adds visual path/status fields: `visual_model_loaded`, `visual_model_path`, `visual_model_load_error`,
  - macOS visual load now tries candidate chain (`configured/source model` -> `same-stem .usdz` -> `canonical .glb`) to avoid SceneKit format dead ends,
  - pose binding is now pre-configured immediately after visual model load; readiness is observable without requiring first pointer event tick.
- Mouse Companion WebSettings diagnostics panel is now wired in section UI:
  - `Mouse Companion` card now renders runtime read-only diagnostics (`model/visual/action/appearance/pose-binding` booleans, `skeleton_bone_count`, runtime/visual model paths, load errors),
  - diagnostics panel values are driven from `/api/state.mouse_companion_runtime`, enabling direct pass/fail checks without reading host logs.
- Mouse Companion runtime proof script is now available for repeatable feasibility checks:
  - command: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-mouse-companion-proof.sh --skip-build --skip-webui-build`,
  - assertion gate: `model_loaded && visual_model_loaded && action_library_loaded && appearance_profile_loaded && pose_binding_configured && skeleton_bone_count>0`.
- Mouse Companion P9 third slice (action switch observability + regression route) is now wired:
  - `mouse_companion_runtime` adds action snapshot fields: `last_action_code`, `last_action_name`, `last_action_intensity`, `last_action_tick_ms`,
  - test-only route `/api/mouse-companion/test-dispatch` is available behind `MFX_ENABLE_MOUSE_COMPANION_TEST_API=1` for deterministic event injection (`status/move/button_down/button_up/click`),
  - proof script now executes and asserts action sequence `idle -> follow -> drag -> follow -> click_react` in addition to model/action/appearance/pose readiness checks.
- Mouse Companion P9 fourth slice (skeleton-action contract coverage report) is now wired:
  - new analyzer module: `MouseFx/Core/Pet/PetActionCoverageAnalyzer.{h,cpp}` computes expected-action coverage (`idle/follow/click_react/drag`), mapped track counts, missing actions, and missing bone-track list,
  - `/api/mouse-companion/test-dispatch` now returns `action_coverage` (ready/error + per-action coverage + missing bones),
  - proof script now gates on coverage contract (`expected=4`, `missing_actions=0`, `mapped_track_count>0`) while preserving action-sequence assertions.
- Mouse Companion P9 fifth slice (bone remap contract) is now wired:
  - action library supports optional top-level `bone_remap` (`trackBone -> targetBone | [targetBones...]`),
  - runtime clip binding and coverage analyzer share the same remap resolution logic, so diagnostics and runtime behavior stay consistent,
  - sample `pet-actions.json` now includes `Chest -> Spine` remap and proof gate reaches full coverage (`overall_coverage_ratio=1.0`, `missing_bone_names=[]`).
- Mouse Companion P9 sixth slice (coverage observability in `/api/state` + Web diagnostics) is now wired:
  - `/api/state.mouse_companion_runtime.action_coverage` now includes `actions[]` (per-action `clip_present/track_count/mapped_track_count/coverage_ratio/missing_bone_tracks`),
  - runtime caches coverage report at action-library load time and resets cache on model/action disable/failure paths to avoid stale diagnostics,
  - Web `Mouse Companion` diagnostics panel now renders action-level coverage detail lines directly from `/api/state` (no test route dependency),
  - proof script now consumes coverage gate from `/api/state.mouse_companion_runtime.action_coverage` (action sequence assertions remain on test route).
- Mouse Companion visual follow coordinate regression fix is applied:
  - macOS pet visual host update now converts runtime cursor point from Quartz to Cocoa before calling Swift window-position update,
  - pet window origin is clamped to visible desktop bounds on every follow tick, and `show()` now starts from main-screen visible center as a safe anchor,
  - this removes coordinate-space mismatch/off-screen drift that could make companion invisible while runtime diagnostics still reported `visual_model_loaded=true`.
- Mouse Companion visual loader robustness is strengthened (macOS Swift bridge):
  - visual model load now requires at least one renderable geometry node to avoid false-positive "loaded but invisible" states,
  - normalization now clamps `baseScale` and applies a safe fallback when model bounds are invalid/degenerate.
- Mouse Companion visual framing robustness is strengthened (macOS Swift bridge):
  - model normalization now recenters X/Y/Z (not Y-only), reducing cases where assets stay outside camera frustum after load,
  - camera placement now adapts to normalized model size (`scaledMaxDim`) to reduce "runtime loaded but viewport empty" risk across heterogeneous assets.
- Mouse Companion visual host view-frame bug is fixed (macOS Swift bridge):
  - `SCNView` and `contentView` now use content-local bounds (`0,0,width,height`) instead of window-positioned frame coordinates,
  - this prevents SceneKit canvas from being laid out outside the window content area (a direct cause of "runtime loaded but nothing visible").
- Mouse Companion visual presentation tuning is updated (macOS Swift bridge):
  - default facing yaw is corrected to front-facing (`pi`) so companion no longer appears back-facing by default,
  - canvas sizing now adds safe padding at small configured sizes (`max(160, size+48)`) to reduce cramped framing,
  - lighting setup is retuned to a softer toon-style directional+ambient+rim stack (with shadows disabled) to reduce harsh/awkward shading,
  - runtime sway now applies relative to normalized base position (no longer overwriting centered model offset each frame).
- Mouse Companion visual framing/visibility tuning is refined (macOS Swift bridge):
  - canvas minimum/padding is increased further (`max(240, size+128)`) to reduce top/bottom clipping on chibi proportions,
  - model default pose is now front-facing immediately after load (before first mouse-move update),
  - camera distance now uses FOV-based geometric fit for normalized model diameter (instead of fixed linear factor),
  - material pass now forces double-sided rendering for pet meshes to avoid ears/feet disappearing under backface culling.
- Mouse Companion camera zoom regression is corrected (macOS Swift bridge):
  - camera FOV widened to `62` and fit-distance multiplier/min-distance increased, reducing close-up face cropping,
  - normalization base scale lowered (`0.72/maxDim`) so default framing leaves more full-body headroom.
- Mouse Companion edge-follow behavior is refined (macOS Swift bridge):
  - window boundary clamp is now "soft edge" (allows partial out-of-screen overflow) instead of strict full-in-view clamp,
  - this removes the near-edge "stuck" feeling when cursor keeps moving outward while companion previously saturated at desktop bounds.
- Mouse Companion edge constraint asymmetry is fixed (macOS Swift bridge):
  - follow anchor now tracks companion center on X (instead of left-edge origin), removing left/right boundary asymmetry near screen edges,
  - desktop clamp bounds now use full `NSScreen.frame` union (not `visibleFrame`), so Dock/menu-bar safe-area no longer behaves like a hard motion boundary.
- Mouse Companion edge saturation is further reduced (macOS Swift bridge):
  - soft-edge overflow budget is widened to `>= max(1.25*window_size, 0.90*window_size + |offset|)`,
  - this avoids early clamp saturation when cursor keeps moving outward near desktop boundaries.
- Click effect regression guard is tightened:
  - `DispatchRouter::OnClick` no longer suppresses click rendering via hold-policy gate,
  - click lane now always attempts normal wasm/native rendering when effects are not blacklisted, preventing stale hold state from globally swallowing click effects.
- Effects `none` behavior regression is fixed for all five categories (`click/trail/scroll/hold/hover`): selecting `none` and pressing `Apply` now persists and keeps runtime disabled, instead of being normalized back to default effects on refresh.
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
