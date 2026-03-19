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
  - `MouseCompanionPluginHostV1` builtin plugin bridge is now active in parallel: typed `PetInputEvent / PetRuntimeConfig / PetPoseFrame` plus `Initialize / OnInput / Tick / SamplePose / OnConfigChanged / Shutdown` host surface are live, and the builtin native plugin now receives `AppController`'s resolved action/head-tint/pose-frame snapshot each frame.
  - Current boundary: v1 builtin plugin is now a real unified snapshot sink, but it still does not drive visible pet rendering yet; Phase0 diagnostics remain the stable user-facing runtime contract until final render-path migration starts.
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
  - Blacklist conflict boundary (important): `pet` is not uniformly coupled to the regular mouse-effect blacklist. Today `pet` still consumes some pointer lanes before `IsEffectsBlockedByAppBlacklist()` (for example move/scroll/button and shared visual-frame decay), while several hold/effect lanes are still gated by the blacklist. Treat `pet` vs `effects` blacklist policy as an explicit per-lane contract when changing routing, otherwise regressions are easy to introduce.
  - Runtime action updates are forwarded: `idle/follow/click_react/drag/hold_react/scroll_react`.
  - Click visual profile remains tauri-style `in-hold-out` envelope; SceneKit click pose is now press-down/squash-rebound (no click head-twist dominant pose).
  - Click clip reset parity (active): SceneKit now restores all nodes touched by `clickReact` tracks back to their cached rest transforms on every frame before re-sampling the one-shot clip, reducing residual pose carry-over and making the press/rebound silhouette closer to tauri's per-frame `resetPose -> applyActionPose`.
  - Click core-bone resolution is now closer to tauri: SceneKit resolves `Head` first, then prefers a matching chest/spine ancestor above generic name-only fallback so `clickReact` chest squash is less likely to hit a lower body/root node and lift the legs.
  - Click eligibility no longer uses the legacy "missing release snapshot still accept click" compatibility fallback; native now requires a captured press/release snapshot before accepting `clickReact`, matching tauri's stricter gesture gate more closely.
  - Idle/hover parity (active): native follow selection no longer stays latched in `follow` while stationary. `TickPetVisualFrame()` now mirrors tauri's `moveSpeed` selection model: `face_pointer_enabled=true` and pointer speed above threshold => `follow`; otherwise => `idle`, with exponential pointer-speed decay on frame ticks (`8.5/s` prod, `6.0/s` test; threshold `45px/s` prod, `30px/s` test).
  - Hover mapping now follows the roadmap contract (`hover -> idle`): `hover_start/hover_end` remain observable plugin inputs, but pet visual selection stays on the continuous `idle/follow` chooser instead of inventing a separate hover action.
  - Hover blacklist boundary (active): pet hover entry now starts before regular effect blacklist gates; blacklisted apps still block hover wasm/native effects, but pet idle/hover visual state continues to work.
  - macOS visual host now gives `idle` a visible hover-state boost: when hover is active, idle intensity increases subtle bob/ear/hand motion for both the placeholder view and the SceneKit model without touching click semantics.
  - SceneKit action-library sampling is no longer click-only: the macOS visual host now parses loop clips for `idle/follow` in addition to `clickReact`, and applies them on the real model before idle procedural augmentation.
  - Real-model idle ear parity (active): SceneKit now mirrors tauri's idle procedural layer on top of the `idle` clip by driving ear Z-wave and hand sway against cached rest transforms; this fixes the previous state where the real Usagi model stayed almost static in idle because only click clips were sampled. The ear/hand procedural amplitudes now use tauri-style fixed idle amplitudes (no extra attenuation by hover idle intensity), because the previous native path was visually under-driving ear motion even after `idle` clip sampling was restored.
  - Pose binding for the real model is now semantic instead of exact-name-only: `left/right ear/hand/leg` bindings can resolve Blender-style bones like `DEF-Ear.L`, `DEF-Arm.L.*`, `DEF-Leg.L.*`, preventing idle procedural motion from disappearing just because the asset uses different exact bone names.
  - Idle ear jitter fix (active): semantic pose binding now selects a single representative bone for each logical binding (for example the root ear bone) instead of rotating the entire matched Blender export chain at once; this removes compounded idle ear motion while keeping blacklist-decoupled pet hover/idle routing unchanged.
  - Idle root-motion alignment (active): the real-model SceneKit path no longer adds extra idle root pitch/yaw wobble on top of the `idle` clip + ear procedural layer. Idle keeps light body translation bob, but root rotation now stays closer to tauri so ear motion is less likely to read as high-frequency jitter.
  - Pose write conflict guard (active): SceneKit now treats all-zero semantic pose packets as no-op updates (with one-time rest clear after non-zero), so idle ear procedural motion is no longer reset every frame by empty pose packets. `apply_pose` dispatch is also main-thread deterministic (main direct / non-main sync) to avoid async queue jitter between pose writes and frame updates.
  - Hold parity stage1 (active): native semantic hold pose now moves closer to tauri's `applyHoldProcedural` silhouette. SceneKit semantic pose increases ear forward fold (`x≈-1.02`), hand inward fold (`x≈-0.34`, `y/z≈0.42`), and leg curl (`z≈16deg`) instead of the previous weak lift-only hold pose; the placeholder fallback now also compresses body/head and strengthens limb curl during hold.
  - Hold parity stage2 (active): pet visual hold timing is now partially decoupled from the generic hold-effect timer. Pet visual selection enters `holdReact` on stable press (`130ms` prod / `90ms` test, stable speed `<=24px/s` prod / `<=30px/s` test), keeps a release buffer from `mouse_companion.release_hold_ms`, and suppresses hold immediately after scroll (`720ms`) so the pet no longer waits entirely for the shared `hold` lane before reacting.
  - Scroll parity stage1 (active): native scroll visual motion now uses a tauri-style impulse envelope instead of simple linear decay. SceneKit and fallback both run a `0.72s` scroll pulse (`0.56s` test) with `inRatio=0.42` / `holdRatio=0.16`, amplitude tracking, and light flap modulation for hands/legs; the real-model ear root rotation signs were flipped to match Usachi's actual ear-bone local axes, so tauri-style `wingOpen + flap` now opens outward instead of collapsing inward.
  - Follow parity stage1 (active): macOS visual host now tracks pointer X while in `position_mode=follow` and applies tauri-style facing yaw (`baseYaw - normalizedX * 0.28`) to the real model instead of keeping a constant facing direction. The fallback view also uses the same pointer-normalized X signal for lighter horizontal follow tilt, and native follow forward-lean has been reduced so the result reads as follow rather than drag.
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
- Manual websettings runner lock handoff (active):
  - `run-macos-core-websettings-manual.sh` now preempts stale lock owners from prior manual websettings runs before acquiring `mfx-entry-posix-host` lock.
  - lock wait now defaults to `MFX_MANUAL_ENTRY_LOCK_TIMEOUT_SECONDS=30` (manual-runner scoped) to avoid long silent stalls from inherited global lock timeout settings.
  - 2026-03-19 startup deadlock fix: `mfx_macos_mouse_companion_panel_apply_pose_v1` switched from `DispatchQueue.main.sync` to `DispatchQueue.main.async` to break main-thread <-> dispatch-worker circular wait during tray bootstrap (`GetConfigSnapshot` vs pose tick).

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
