# Agent Current Context (P1, 2026-03-19)

## Purpose
- This file is the compact execution truth for daily work.
- Keep only active contracts and operational facts.
- Move detailed history/design discussion to P2 docs.

## Scope and Priority
- Primary host: macOS.
- Delivery order: macOS first, Windows regression-free, Linux compile/contract-level.
- macOS stack rule: new capability modules are Swift-first; avoid expanding `.mm` for new large modules.

## Runtime Lanes
- Stable lane: `scaffold`.
- Progressive lane: `core` (`mfx_entry_posix_host`).
- Policy: new cross-platform capability lands in `core` first, then backports as needed.

## Active Product Goals
- Keep wasm runtime bounded-but-expressive (host-owned render boundaries).
- Keep plugin lanes decoupled (`effects` vs `indicator`) with explicit diagnostics.
- Keep automation mapping accurate/observable with low regression risk.
- Rebuild `mouse_companion` in plugin-first route with click-first visible parity.

## Capability Snapshot

### Visual Effects / WASM
- `click/trail/scroll/hold/hover` are active in `core`.
- Shared command tail (`blend_mode/sort_key/group_id`) is active.
- Group-retained model is active; transform/material/pass remain host-owned.

### Input Indicator
- macOS/Windows label/streak semantics aligned (`L xN`, `W+ xN`).
- Indicator wasm dispatch has dedicated lanes and budget floor.
- `/api/wasm/load-manifest` auto-infers surface and avoids cross-surface misload.
- `SetInputIndicatorConfig` syncs runtime host immediately after apply.
- Stale indicator manifest fallback is active: missing manifest degrades to native mode.

### Plugin Management / WebUI
- Unified top-level `Plugin Management` section is active.
- WebUI apply flow is backend-state-driven (post-apply reconcile + refresh).
- Sidebar order is fixed: `General -> Mouse Companion -> Cursor Effects -> Input Indicator -> Automation Mapping -> Plugin Management`.

### Mouse Companion (Current Truth)
- Backend reset was executed; old skeleton runtime remains removed.
- Plugin-first landing route is active (Phase0 -> Phase1 -> Phase2).
- Phase0 host scaffold is active:
  - `MouseCompanionPluginHostPhase0` integrated in controller dispatch/lifecycle.
  - Runtime diagnostics expose plugin-host fields (`plugin_host_ready`, `active_plugin_id`, `compatibility_status`, `plugin_event_count`).
- Phase1 click-first backend semantics are active:
  - click gate: `press<=220ms && travel<=10px` with scroll suppression window.
  - `fixed_bottom_left` move path stays `idle` (no follow transition).
  - `button_down` no longer forces `drag`; drag now starts only after pointer travel while pressed.
  - click streak/head-tint runtime diagnostics update with decay.
  - click pulse/intensity tuned for tauri parity: base `0.84` + streak step `0.05` (test profile `0.92 + 0.06`).
  - visual motion profile is unified for `click/hold/scroll` with prod/test dual presets (`mouse_companion.use_test_profile` switch).
- Phase1 macOS visual host is active (model-first with placeholder fallback):
  - Swift bridge: `Platform/macos/Pet/MacosMouseCompanionPhase1Bridge.swift`.
  - Enabled config creates native panel and supports `fixed_bottom_left` + `follow` position modes.
  - Visual host now tries `Assets/Pet3D/source/pet-main.*` first (`.usdz` preferred when SceneKit loads), then falls back to `phase1://placeholder/usagi`.
  - Visual host now loads `action_library_path` into a built-in clip sampler; `clickReact` is sampled from `pet-actions.json` (binary-search keyframe + quaternion slerp + bone_remap).
  - SceneKit model path now has its own 60fps frame driver; click one-shot is no longer tied to sparse input-event ticks.
  - Imported model animations are cleared on load to avoid built-in clip interference with click parity.
  - When SceneKit loads the `.usdz` fallback, anonymous skeleton nodes are hydrated from sibling `.glb` joint metadata so `head/chest/ear/arm/leg` mappings still resolve.
  - SceneKit framing work remains in-progress for Usagi full-body fit.
  - Current known regression (active): `projectedRenderableBounds` can under-report rendered height for the `.usdz` path, causing panel aspect to drift toward square and clipping ear/head-root/foot extents.
  - Load-time fit now avoids oversized multiplier compensation and runs once on the first rendered frame, preferring snapshot bounds with alpha filtering and falling back to projection bounds when snapshot is unavailable.
  - Keep panel debug border only as temporary observability aid while tuning framing.
  - Default facing pitch is now neutral (`x = 0.0`) so the pet is front-facing at rest (no baseline forward tilt).
  - Click smoothness tuning (active): model frame loop now runs at `120fps` local cadence (`1/120` timer), click `dt` cap is reduced to `0.05`, and click chest squash/rebound uses cubic smoothstep easing instead of linear interpolation.
  - Click trigger semantics (active): SceneKit click one-shot now restarts immediately on every incoming `clickReact` event instead of batching through a pending-trigger counter; the one-shot restarts from `t=0` so rapid clicks do not get visually merged away.
  - Head tint parity (active): SceneKit now mirrors tauri's click tint selection model by resolving head-tint target meshes first (name match, then upper-half fallback, then topmost mesh fallback) and blending only those materials toward a red tint color instead of applying a weak whole-body multiply.
  - Head tint decay parity (active): pet visual frame ticks now refresh `clickStreak.tintAmount` on the shared dispatch timer even without new mouse input, so redness fades continuously and the next click accumulates from the current remaining tint instead of snapping from stale event-only values.
  - Click streak tint contract is now tauri-aligned: tint holds steady during the active streak window (`breakMs`), only starts decaying after the streak resets, and a new click after `breakMs` still adds on top of the current remaining tint instead of clearing tint back to zero first.
  - Pet visual frame ticks now run before wasm/effect suppression gates on the shared timer, so head-tint fade continues even when the foreground window blocks regular effects routing.
  - Runtime action updates are forwarded: `idle/follow/click_react/drag/hold_react/scroll_react`.
  - Click visual profile remains tauri-style `in-hold-out` envelope; SceneKit click pose is now press-down/squash-rebound (no click head-twist dominant pose).
  - Click clip reset parity (active): SceneKit now restores all nodes touched by `clickReact` tracks back to their cached rest transforms on every frame before re-sampling the one-shot clip, reducing residual pose carry-over and making the press/rebound silhouette closer to tauri's per-frame `resetPose -> applyActionPose`.
  - Click core-bone resolution is now closer to tauri: SceneKit resolves `Head` first, then prefers a matching chest/spine ancestor above generic name-only fallback so `clickReact` chest squash is less likely to hit a lower body/root node and lift the legs.
  - Click eligibility no longer uses the legacy "missing release snapshot still accept click" compatibility fallback; native now requires a captured press/release snapshot before accepting `clickReact`, matching tauri's stricter gesture gate more closely.
  - SceneKit click window now hard-isolates semantic pose injection (`apply_pose` suppressed while click one-shot is active) to prevent legacy-style blended motion.
  - C++ semantic pose stream now drives `hold/scroll` only; click pose injection path is removed.
  - SceneKit drag yaw is intensity-scaled (no fixed large turn at low-intensity micro-move).
  - Pose channels are forwarded to both SceneKit bones and placeholder parts.
  - `/api/state.mouse_companion_runtime` reports `model_loaded` + `loaded_model_path` when real model is active; otherwise keeps placeholder path.
- Non-goal for current phase:
  - action-library/effect-profile/appearance runtime recovery (scheduled for later phase).

### Automation Mapping
- App-scope normalization/parser contracts are stable.
- Preset/custom gesture mapping with threshold and ambiguity rejection is active.
- Trigger button supports `none`; `Draw -> Save` custom gesture flow is active.
- macOS shortcut capture/injection punctuation path is aligned (`BracketLeft/BracketRight/...`).

## Observability and Debug Contract
- Runtime diagnostics are gated by debug mode where required.
- Default non-debug run avoids high-volume debug lanes.
- WebUI debug polling is adaptive and focus-aware.
- Mouse companion test route is available behind `MFX_ENABLE_MOUSE_COMPANION_TEST_API=1`.

## Regression Gates
- Canonical regression entry:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Mouse companion manual proof helper:
  - `tools/platform/manual/run-macos-mouse-companion-proof.sh`

## Contracts That Must Not Drift
- Keep stdin JSON command compatibility.
- Keep current wasm ABI compatibility unless migration is explicitly approved.
- Keep host-owned bounded rendering strategy; no raw shader ownership without architecture approval.
- Keep docs synchronized in the same change set for every behavior/contract change.

## P2 Routing (Read on Demand)
- P2 index:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
- Mouse companion plugin roadmap:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-plugin-landing-roadmap.zh-CN.md`
- Mouse companion backend reset contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-backend-reset-contract.zh-CN.md`
- Input indicator capability references:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/input-indicator-cross-platform-contract.md`
- Automation behavior notes:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/automation-mapping-notes.md`
- Server structure:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/server-structure.md`
- Regression workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`

## Documentation Governance
- `current.md` is P1-only and must stay compact.
- Keep long history in P2 docs and archives, not in this file.
- Context artifacts:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-index.json`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-map.md`
