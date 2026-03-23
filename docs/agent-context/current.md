# Agent Current Context (P1, 2026-03-23)

## Purpose
- This file is the compact execution truth for daily work.
- Keep only active contracts, current boundaries, and route pointers.
- Move implementation history and detailed rollout notes to P2 docs.

## Scope And Priority
- Primary host: macOS.
- Delivery order: macOS first, Windows regression-free, Linux compile/contract-level.
- macOS stack rule: new capability modules are Swift-first; avoid expanding `.mm` for large new modules.
- Windows VS2026 direct project build is healthy; any remaining `MFCMouseEffect.slnx` `ValidateSolutionConfiguration` issue is solution-metadata work, not a C++ compile regression.
- Cross-machine workflow: macOS is the development source, Windows is the synced validation workspace via `Syncthing`, and Windows handoff should default to `F:\language\cpp\code\MFCMouseEffect`.

## Active Product Goals
- Keep wasm runtime bounded-but-expressive with host-owned render boundaries.
- Keep plugin lanes decoupled (`effects` vs `indicator`) with explicit diagnostics.
- Keep automation mapping accurate and observable with low regression risk.
- Rebuild `mouse_companion` on a plugin-first route with click-first visible parity.

## Capability Snapshot

### Visual Effects / WASM
- `click / trail / scroll / hold / hover` are active in `core`.
- Shared command tail (`blend_mode / sort_key / group_id`) is active.
- Group-retained model is active; transform/material/pass remain host-owned.
- Windows blacklist routing root fix is active: pointer suppression resolves the process at the current screen point first, and trail synthetic-follow is limited to a short post-input smoothing window.
- Cross-platform click ripple baseline is active: Windows now honors `EffectConfig.ripple`; default click is shorter, smaller, center-clear, softer-glow, and single-ring.

### Input Indicator
- macOS/Windows label and streak semantics are aligned (`L xN`, `W+ xN`); indicator wasm dispatch has dedicated lanes, auto-inferred surface loading, immediate runtime sync on apply, and clean native fallback on missing/stale manifests.

### Plugin Management / WebUI
- Unified top-level `Plugin Management` section is active.
- WebUI apply flow is backend-state-driven (`post-apply reconcile + refresh`).
- Settings launch lifecycle is shared through `WebSettingsLaunchCoordinator`; platform shells still keep their own `OpenUrlUtf8(...)`.
- First uncached WebUI reload now fetches `/api/state` and `/api/schema` in parallel.
- Runtime settings page currently uses checked-in `WebUI/settings-form.js` and `WebUI/mouse-companion-settings.svelte.js`, not `WebUIWorkspace` source files directly.
- When `mouse-companion` is the initially visible section and it has not rendered yet, `settings-form.js` now defers the first Mouse Companion render to the next animation frame to reduce first-paint blocking.
- Sidebar order is fixed:
  - `General -> Mouse Companion -> Cursor Effects -> Input Indicator -> Automation Mapping -> Plugin Management`

### Mouse Companion
- Backend reset remains in effect; old skeleton runtime stays removed.
- Plugin-first landing route is active (`Phase0 -> Phase1 -> Phase2`).
- Shared `IPetVisualHost` + `PlatformPetVisualHost` abstraction is the stable cross-platform visual-host seam.

#### Windows Pet
- Current stage: `Phase1.5`.
- Current gap vs macOS: Windows still renders a stylized preview contract, not the real 3D model path.
- Shared placement contract is active: `relative`, `absolute`, legacy `fixed_bottom_left`, `strict / soft / free`, and target-monitor resolution.
- Active backend path: `window -> backend factory/registry -> renderer input -> renderer runtime -> scene builder -> painter`.
- Windows appearance validation supports:
  - built-in `activePreset` values in `pet-appearance.json`
  - dedicated combo-only synced JSONs for receive-only Windows validation
  - runtime diagnostics for `appearance_requested_preset_id / appearance_resolved_preset_id / appearance_skin_variant_id / appearance_accessory_family / appearance_combo_preset`
- Combo-persona acceptance is explicit:
  - first set is `cream+moon`, `night+leaf`, `strawberry+ribbon-bow`
  - acceptance should be recorded as `pass (dynamic-biased)` when static readability is weaker than dynamic readability
  - native `.cmd`, PowerShell, and Git Bash paths are aligned

#### Windows Pet Renderer / Plugin Lane
- `Win32MouseCompanionRenderPluginHost` is the current Windows-first seam for renderer-owned appearance/persona semantics.
- Stable provider today is still `builtin native`.
- Windows renderer-plugin env entry:
  - `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN`
  - `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST`
- wasm request contract:
  - manifest must target `effects`
  - manifest must enable `frame_tick`
  - failure falls back to builtin immediately
- wasm preflight/load failures are normalized to machine-readable codes; do not depend on free-form text in tests.
- Optional sidecar metadata path is `<manifest>.mouse_companion_renderer.json`.
- Sidecar must declare:
  - `schema_version >= 1`
  - `renderer_lane = mouse_companion_renderer`
  - `supports_appearance_semantics = true`
  - `appearance_semantics_mode = builtin_passthrough|wasm_v1`
- Runtime plugin diagnostics surface at least:
  - `appearance_plugin_id`
  - `appearance_plugin_kind`
  - `appearance_plugin_source`
  - `appearance_plugin_selection_reason`
  - `appearance_plugin_failure_reason`
  - `appearance_plugin_manifest_path`
  - `appearance_plugin_runtime_backend`
  - `appearance_plugin_metadata_path`
  - `appearance_plugin_metadata_schema_version`
  - `appearance_plugin_appearance_semantics_mode`

#### Windows `wasm_v1` Semantics Summary
- `builtin_passthrough` keeps host-generated semantics and only accepts bounded tuning plus optional `combo_preset_override`.
- `wasm_v1` is the first bounded renderer-owned semantics patch lane.
- `wasm_v1` host apply order is fixed:
  - `theme`
  - `shape` (`frame / face / appendage`)
  - `motion`
  - `mood`
- Current `wasm_v1` patch categories:
  - `theme`: glow/body/head/accent/accessory/pedestal color family
  - `shape`: body/head proportions, muzzle/forehead/whisker detail, ear/tail/follow-ear/click-ear detail
  - `motion`: `follow / click / drag / hold / scroll` main action multipliers
  - `mood`: glow/accent/shadow/pedestal tinting and alpha lanes, plus `hold / drag / scroll / follow` overlay emphasis
- Current design intent:
  - keep the lane bounded and host-owned
  - prefer adding high-value fields over reopening builder-local hardcoding
  - do not turn `wasm_v1` into an unrestricted free-form renderer ABI yet
- Checked-in `wasm_v1` sidecar samples are now also curated by readability intent:
  - default sample: balanced default-candidate baseline
  - agile sample: cooler / sharper `follow / drag`
  - dreamy sample: brighter / floatier `follow / scroll`
  - charming sample: rounder / warmer `click / hold`
- checked-in sidecars now declare `style_intent` and `sample_tier`, and `wasm_v1` also covers `face.brow_tilt_scale / mouth_reactive_scale`, `appendage.follow_leg_stance_scale / hold_leg_stance_scale / drag_hand_reach_scale`, `motion.body_forward_scale`, and `mood.pedestal_tint_mix_scale / click_ring_alpha_scale`, so host/runtime no longer rely only on combo-preset inference and `follow / drag / hold / click` gain stronger structure + mood readability on Win pet.

#### Windows Bring-Up / Validation
- Dedicated native validation entrypoints exist for combo-persona acceptance, renderer-sidecar smoke, renderer-sidecar `wasm_v1` smoke, and renderer lane matrix (`builtin -> builtin_passthrough -> wasm_v1`).
- Renderer lane matrix now also accepts `-WasmV1Style default|agile|dreamy|charming`, so the third lane can switch between the checked-in curated `wasm_v1` samples without manual sidecar replacement.
- Renderer lane matrix now also accepts `-AllWasmV1Styles`, which expands the third lane into `wasm_v1_default / wasm_v1_agile / wasm_v1_dreamy / wasm_v1_charming` and emits separate proof artifacts for each style.
- Checked-in samples exist at `tools/platform/manual/lib/windows-mouse-companion-renderer-sidecar.sample.json` and `tools/platform/manual/lib/windows-mouse-companion-renderer-sidecar.wasm-v1.sample.json`.
- Lane matrix now emits per-lane proof json plus `summary.json`, `summary.md`, and `observation-template.md`; summary also emits:
  - compact lane verdicts
  - per-lane default-lane snapshots
  - per-lane configured sample metadata
  - per-lane runtime sample-tier snapshot
  - compare-vs-builtin summary
  - per-lane `style` + `style_focus_profile`
  - conservative `recommended_default_lane`
  - `recommendation_style_intent` / `recommendation_style_focus_profile` / `recommended_sample_path`
  - `rollout_contract_status`
- Default lane rollout contract: machine summary may nominate a candidate, but actual default switch still requires later manual confirmation.
- Runtime diagnostics now expose both lane and scene-runtime state directly: `default_lane_candidate / source / rollout_status / style_intent / candidate_tier`, `appearance_plugin_sample_tier`, `appearance_plugin_contract_brief`, plus `scene_runtime_adapter_mode / pose_sample_count / bound_pose_sample_count / model_scene_adapter_brief / model_node_adapter_influence / model_node_adapter_brief / pose_adapter_influence / pose_readability_bias / pose_adapter_brief`; adapter modes are fixed to `runtime_only | pose_unbound | pose_bound`.
- `pose_bound` is now visible across appendage, motion/head-body, frame/face anchors, overlay/grounding, and painter readability: beyond geometry, it can also raise shadow/pedestal alpha, strengthen accessory stroke/fill, and make pose badges/overlays read more confidently than `runtime_only / pose_unbound`.
- That same pose-adapter profile is now surfaced through runtime, `/api/state`, test routes, WebUI Runtime Diagnostics, render-proof, and lane matrix, so later 3D-model bring-up can judge pose-lane consumption from one shared contract instead of scattered raw fields; `render-proof` can now also assert adapter mode / brief / minimum influence / minimum readability directly.
- `SceneRuntime` now owns the shared `poseAdapterProfile`; builders and backend diagnostics consume that cached profile directly instead of recomputing influence/readability per file, which keeps the upcoming model-driven seam on one adapter contract.
- `SceneRuntime` now also owns `modelSceneAdapterProfile`, exposing `seamState / seamReadiness / brief`; this lets Windows real preview report whether a frame is still preview-only, merely asset-stub-ready, already pose-sampling-ready, or pose-bound-preview-ready for later model-node consumption.
- `SceneRuntime` now also owns `modelNodeAdapterProfile`, so frame/face/adornment/overlay/grounding consume one cached node-offset seam instead of recomputing local pose averages; runtime/proof/matrix/WebUI surface `scene_runtime_model_node_adapter_influence`, `scene_runtime_model_node_adapter_brief`, and `scene_runtime_model_node_channel_brief`.
- `SceneRuntime` now also owns `modelNodeGraphProfile`, `modelNodeBindingProfile`, and `modelNodeSlotProfile`, so Windows preview already walks `node channel -> node graph -> binding entry -> slot` before painter geometry; runtime/proof/matrix/WebUI surface `scene_runtime_model_node_graph_*`, `scene_runtime_model_node_binding_*`, and `scene_runtime_model_node_slot_*`.
- `SceneRuntime` now also owns `modelNodeRegistryProfile` and `assetNodeBindingProfile`, lifting slots into stable future asset-node names and paths; runtime/proof/matrix/WebUI surface `scene_runtime_model_node_registry_*` and `scene_runtime_asset_node_binding_*`, while frame/adornment/overlay already consume registry/binding weight.
- `SceneRuntime` now also owns `assetNodeTransformProfile` and `assetNodeAnchorProfile`, lifting asset-node paths into a minimal transform table and then shared `body/head/appendage/overlay/grounding` anchors; frame/face/adornment/overlay now consume those shared seams and runtime/proof/matrix/WebUI surface both `scene_runtime_asset_node_transform_*` and `scene_runtime_asset_node_anchor_*`.
- `SceneRuntime` now also owns `assetNodeResolverProfile` and `assetNodeParentSpaceProfile`, lifting those local transforms into shared parent-aware node tables before anchor generation; frame/face/adornment/overlay now consume resolver/parent-space seams instead of re-deriving hierarchy drift locally, and runtime/proof/matrix/WebUI surface `scene_runtime_asset_node_resolver_*` and `scene_runtime_asset_node_parent_space_*`.
- Sidecar smoke presets now also assert `default_lane_style_intent` and `appearance_plugin_sample_tier`; `renderer-sidecar-wasm-v1-smoke` accepts `-WasmV1Style default|agile|dreamy|charming`, and `render-proof` now exposes `default_lane_summary`, `default_lane_candidate_tier`, and `appearance_plugin_contract_brief` in both console summaries and saved JSON.
- Lane matrix recommendation now prefers runtime `default_lane_candidate_tier` first, then `sample_tier`, then runtime `default_lane_style_intent`; `observation-template.md` also pre-fills `candidate_tier`, `runtime_default_lane_brief`, and `recommended_sample_tier`, so final manual decisions stay on the same contract vocabulary as runtime and summary.
- `default_lane_candidate_tier` currently uses short machine values to distinguish runtime recommendation semantics: `builtin_shipped_default`, `baseline_reference_candidate`, `ship_default_candidate`, `experimental_style_candidate`, `unclassified_candidate`.
- Lane matrix also derives `style_focus_profile` to summarize the intended motion emphasis: `builtin_control`, `baseline_passthrough_reference`, `balanced_all_rounder`, `follow_drag_tension`, `follow_scroll_float`, `click_hold_warmth`, `unclassified_focus`.
- Mouse Companion WebUI mirrors runtime lane state in `Runtime Diagnostics`, including a short `Lane Verdict`, `Style Intent`, `Candidate Tier`, `Sample Tier`, and `Contract Brief`.

#### Windows Renderer Backend / Preview
- Backend selection diagnostics are active for preference source/name, selected backend, selection/failure reasons, available/unavailable backends, backend catalog, `real_renderer_preview`, and `renderer_runtime_*`.
- Backend lifecycle seam treats `Start() / IsReady() / LastErrorReason()` as first-class fallback signals.
- Placeholder backend remains the always-ready reference implementation.
- `real` backend has a complete internal preview pipeline, but default selection still keeps it behind rollout gate `MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE`.
- Real-preview dynamic readability is already stronger:
  - `dreamy` biases `follow` toward lighter lift and softer grounding
  - `agile` biases `drag/follow` toward sharper lean and reach
  - `charming` biases `hold/click` toward rounder bounce and softer face/ear response
- Current boundary:
  - visible backend is stable enough for `Phase1.5`
  - the main Windows vs macOS gap is still renderer form factor, not appearance-lane plumbing
  - future real-renderer work should stay behind existing backend/runtime seams

#### macOS Pet
- macOS Phase1 visual host is active:
  - model-first with placeholder fallback
  - `.usdz` preferred when SceneKit can load it
  - runtime action updates are forwarded (`idle / follow / click_react / drag / hold_react / scroll_react`)
- Shared placement contract is active: `relative`, `absolute`, legacy aliases, `strict / soft / free`.
- Runtime `size_px` resize path is active and no longer create-time-only.
- Idle/follow/click/scroll parity direction is active; remaining known boundary is `.usdz` framing on some paths.

### Automation Mapping
- App-scope normalization/parser contracts are stable.
- Preset/custom gesture mapping with thresholding and ambiguity rejection is active.
- Trigger button supports `none`.
- `Draw -> Save` custom gesture flow is active.
- macOS shortcut capture/injection punctuation path is aligned.

## Observability And Debug Contract
- Runtime diagnostics are gated by debug mode where required.
- Default non-debug run avoids high-volume debug lanes.
- WebUI debug polling is adaptive and focus-aware.
- Mouse companion test route remains gated behind `MFX_ENABLE_MOUSE_COMPANION_TEST_API=1`.

## Regression Gates
- Canonical regression entry: `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- macOS daily shortcut: `./mfx run-no-build` / `./mfx fast` skip both core and WebUI rebuilds; `./mfx run` / `./mfx start` perform fresh build preparation.

## Packaging / Startup Truth

### Tray Menus
- macOS tray menu intentionally exposes only `Star Project`, `Settings`, and `Exit`.
- Windows tray menu now follows the same product rule.

### Launch At Startup
- macOS: LaunchAgent uses `tray` mode, explicit toggle rewrites plist and applies `launchctl`, normal startup repairs plist only.
- Windows: uses `HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run`, and current executable path is rewritten idempotently so relocation can self-heal.

### Packaging
- Preferred packaging entrypoint is `./mfx package`.
- Windows installer remains Inno Setup based.
- macOS package output remains `MFCMouseEffect.app`, `Install/macos`, folder + `.zip` + unsigned `.dmg`.
- Current package policy: minimal pet runtime assets only, wasm demo plugin ships runtime files only, packaged host binary is stripped in-bundle, `Install/macos/` is git-ignored, and Gatekeeper/notarization is still deferred.

### Local Dev Sync
- Repository root carries a Syncthing-focused `.stignore`.
- Build outputs, IDE caches, package outputs, dependency caches, and generated `docs/.ai` maps should stay local.

## Contracts That Must Not Drift
- Keep stdin JSON command compatibility.
- Keep current wasm ABI compatibility unless migration is explicitly approved.
- Keep host-owned bounded rendering strategy; no raw shader ownership without architecture approval.
- Keep docs synchronized in the same change set for every behavior or contract change.

## P2 Routing
- P2 index: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
- Windows real-renderer contract: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-real-renderer-contract.md`
- Mouse companion plugin roadmap: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-plugin-landing-roadmap.zh-CN.md`
- Windows manual checklist: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/ops/windows-mouse-companion-manual-checklist.md`
- Server structure: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/server-structure.md`
- Regression workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
