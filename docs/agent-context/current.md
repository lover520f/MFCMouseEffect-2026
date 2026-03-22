# Agent Current Context (P1, 2026-03-21)

## Purpose
- This file is the compact execution truth for daily work.
- Keep only active contracts, current boundaries, and route pointers.
- Move detailed implementation notes to targeted P2 docs.

## Scope And Priority
- Primary host: macOS.
- Delivery order: macOS first, Windows regression-free, Linux compile/contract-level.
- macOS stack rule: new capability modules are Swift-first; avoid expanding `.mm` for new large modules.
- Windows VS2026 project sync (2026-03-21) is healthy again at direct project-build level; treat any remaining `MFCMouseEffect.slnx` `ValidateSolutionConfiguration` failure as solution-metadata work, not a C++ compile regression.
- Windows `appearance combo persona` work now has an explicit real-device validation lane: treat `cream+moon`, `night+leaf`, and `strawberry+ribbon-bow` as the first acceptance set, and record weak static / strong dynamic readability as `pass (dynamic-biased)` instead of silently assuming full static readability.
- Windows combo-persona acceptance now also has a dedicated native `.cmd` entrypoint, so real-device validation no longer depends on retyping the proof command before manual comparison.
- that native combo-persona lane now also auto-checks runtime appearance diagnostics against the synced `pet-appearance.json`, so Win validation can fail fast on preset/persona drift before doing manual visual comparison.
- to respect the mac-send / win-receive-only Syncthing workflow, combo-persona acceptance now switches runtime `appearance_profile_path` across dedicated synced combo-only JSON files instead of asking Win validation to edit the main `pet-appearance.json` in place.
- the Windows Git Bash helper is now aligned with that same combo matrix contract too, so `.cmd/.ps1` and `.sh` no longer diverge on persona acceptance coverage.
- Current cross-machine workflow assumption is now explicit too: macOS is the primary development source, Windows is the synced validation workspace via `Syncthing`, and Windows command handoff should default to the synced path `F:\language\cpp\code\MFCMouseEffect` instead of telling the user to manually copy sources first.
- Windows appearance validation is now also easier to switch manually: `pet-appearance.json` carries built-in `activePreset` values (`default`, `cream-moon`, `night-leaf`, `strawberry-ribbon-bow`), and Windows appearance loading now prefers a valid active preset before falling back to `default`.
- Windows appearance validation is now also easier to prove programmatically: renderer/runtime diagnostics expose `appearance_requested_preset_id`, `appearance_resolved_preset_id`, `appearance_skin_variant_id`, `appearance_accessory_family`, and `appearance_combo_preset`, so Win-side real-preview checks no longer have to infer persona selection only from pixels.
- Windows real preview now also routes appearance/persona expansion through a dedicated renderer-plugin host seam (`Win32MouseCompanionRenderPluginHost`) with a builtin native plugin first; Phase1.5 still ships only the builtin lane, but `appearance_plugin_id/kind/source` are now part of runtime diagnostics so future wasm takeover can reuse the same host-visible contract instead of reopening builder code.
- that Windows-first seam now also has a wasm-adapter skeleton and explicit fallback observability: `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN` + `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST` can request the wasm lane, while runtime diagnostics now also expose `appearance_plugin_selection_reason/failure_reason/manifest_path/runtime_backend` so provider attach failures no longer look like silent renderer drift.
- that same skeleton now also has a minimal manifest preflight contract: the requested wasm manifest must currently target the `effects` surface and keep `frame_tick` enabled, otherwise the host treats it as renderer-plugin attach failure and falls back to builtin immediately.
- the renderer-plugin wasm preflight/load-failure rules are now also split into a dedicated Windows contract file instead of continuing to grow inside `Win32MouseCompanionRenderPluginHost.cpp`, so later ABI expansion can stay isolated from builtin appearance semantics code.
- manifest preflight failure reasons are now normalized to stable machine-readable codes (`renderer_plugin_manifest_io_error`, `renderer_plugin_manifest_json_parse_error`, `renderer_plugin_manifest_invalid`, `renderer_plugin_manifest_missing_effects_surface`, `renderer_plugin_manifest_requires_frame_tick`) so Windows acceptance can assert fallback outcomes without depending on free-form loader text.
- renderer-plugin wasm preflight now also supports an optional sidecar metadata file (manifest replace-extension -> `.mouse_companion_renderer.json`); when present, it must explicitly declare the `mouse_companion_renderer` lane and `supports_appearance_semantics=true`, otherwise the host treats it as metadata-contract failure and falls back to builtin.
- runtime diagnostics now also expose `appearance_plugin_metadata_path` and `appearance_plugin_metadata_schema_version`, so Windows validation can distinguish sidecar-backed renderer-plugin bring-up from legacy manifest-only compatibility mode.
- the first renderer-specific ABI modes are now explicit too: sidecar metadata may declare `appearance_semantics_mode`, and the currently accepted values are `builtin_passthrough` and `wasm_v1`; `builtin_passthrough` keeps delegating appearance semantics generation to the builtin host path, while `wasm_v1` applies a bounded renderer-owned semantics patch on top of the host baseline.
- those modes are no longer diagnostics-only: sidecar metadata may still provide `combo_preset_override`, and `builtin_passthrough` keeps the earlier controlled tuning path, while `wasm_v1` now also accepts an `appearance_semantics` object for bounded `theme/frame/face/appendage/motion/mood` overrides without reopening builder-local hardcoding.
- that `wasm_v1` motion patch is now less partial too: besides `follow_state_lift / click_squash / drag_lean / hold_head_nod`, it now also supports `scroll_tail_lift` and `follow_head_nod`, so the first renderer-owned semantics lane can influence all five main Win-pet action lines instead of stopping short on `scroll` and `follow` nuance.
- that same `wasm_v1` lane now also reaches two more high-value structure cues for Win readability: `face.whisker_spread_scale` and `appendage.follow_tail_width_scale`, so lane deltas no longer have to lean only on motion and color shifts.
- `wasm_v1` now also controls a slightly more stable silhouette pair for Win pet readability: `frame.head_width_scale` and `appendage.follow_ear_spread_scale`, so lane deltas can read even before stronger motion pulses are obvious.
- `wasm_v1` now also reaches two high-value overlay mood channels, `mood.scroll_arc_alpha_scale` and `mood.follow_trail_alpha_scale`, so lane differences can show up not only in body structure and motion but also in the action-atmosphere feedback Windows already draws.
- `wasm_v1` now also reaches the first two theme-readability fields beyond glow/accent: `theme.body_stroke` and `theme.head_fill`, so lane deltas can start influencing the base creature read instead of only overlays and motion semantics.
- sidecar metadata can now also tune a first controlled set of Win-pet motion/expression multipliers (`follow_lift_scale`, `click_squash_scale`, `drag_lean_scale`, `highlight_alpha_scale`, each `0.5~1.5`) so Windows pet parity can keep moving forward through plugin-fed presentation inputs without waiting for a full free-form wasm semantics ABI.
- that controlled sidecar tuning set now also reaches two more high-value motion channels for Win pet readability: `follow_tail_swing_scale` and `hold_head_nod_scale`, both still bounded to `0.5~1.5`.
- that same sidecar tuning set now completes the main Win-pet motion lanes too by adding `scroll_tail_lift_scale` and `follow_head_nod_scale`, so `follow / hold / click / drag / scroll` all now have at least one controlled plugin-fed tuning entry.
- checked-in sample sidecars now exist at `tools/platform/manual/lib/windows-mouse-companion-renderer-sidecar.sample.json` and `tools/platform/manual/lib/windows-mouse-companion-renderer-sidecar.wasm-v1.sample.json`, so Win validation no longer has to author either the first passthrough sidecar or the first `wasm_v1` semantics patch by hand.
- Windows renderer-sidecar validation now also has a dedicated native smoke entrypoint (`run-windows-mouse-companion-renderer-sidecar-smoke.cmd` / `-Preset renderer-sidecar-smoke`), and that preset fails fast on plugin-kind drift, missing sidecar metadata path, or `appearance_semantics_mode` mismatch before the operator has to manually inspect `/api/state`.
- the first `wasm_v1` lane now also has a matching native smoke entrypoint (`run-windows-mouse-companion-renderer-sidecar-wasm-v1-smoke.cmd` / `-Preset renderer-sidecar-wasm-v1-smoke`), so Windows validation no longer has to hand-assemble the `ExpectedAppearanceSemanticsMode=wasm_v1` proof command either.
- Windows renderer validation now also has a dedicated native lane-matrix entrypoint (`run-windows-mouse-companion-renderer-lane-matrix.cmd`), which runs `builtin -> builtin_passthrough -> wasm_v1` in one pass and restores the original sidecar/env state afterward, so Win bring-up can compare all current renderer-semantics lanes without hand-switching files between runs.
- that lane-matrix entry now also prints a compact manual compare hint for `follow / drag / click / hold / scroll`, and the Windows checklist now has a dedicated `Renderer Lane Matrix` section, so lane bring-up no longer ends at proof pass/fail only.
- that same lane-matrix entry now also auto-persists per-lane proof json plus `summary.json` / `summary.md`, so Win pet lane comparison leaves a compact replay artifact even when the operator does not pass an explicit output prefix.
- that persisted lane summary now also emits a compact verdict per lane (`backend/plugin/mode/pass|fail`), so Win lane bring-up can be skimmed from one short line before drilling into the fuller compare notes.
- that same persisted lane summary now also auto-compares `builtin_passthrough` and `wasm_v1` against the `builtin` baseline for `plugin_kind / semantics_mode / combo_preset / selection_reason / failure_reason / metadata_path_present`, so Win validation can spot lane-contract drift before starting the more subjective motion/readability compare.
- that same lane-matrix artifact bundle now also emits `observation-template.md`, so operators can record the final `follow / drag / click / hold / scroll` motion read next to the generated machine summary instead of keeping the human verdict in transient chat or terminal history.
- that observation template now also includes decision slots for `best lane for current Win pet / recommended default lane now / recommended next tuning target`, so a Windows validation pass can end in a concrete lane decision instead of only descriptive notes.
- the lane-matrix summary itself now also emits a conservative machine-side `recommended_default_lane` candidate based on proof pass/fail, empty failure diagnostics, and whether the lane materially differs from builtin; it is intentionally low-confidence and exists to accelerate review, not to bypass the later manual observation step.
- that machine recommendation now also carries `rollout_contract_status`, making the current rule explicit: machine summary may nominate a candidate, but default-lane rollout must still remain pending until the later human observation step explicitly approves the switch.
- that same default-lane decision state is now also exposed directly by the Windows pet runtime diagnostics as `default_lane_candidate / default_lane_source / default_lane_rollout_status`, so future rollout work no longer has to infer default-lane readiness only from external lane-matrix artifacts.
- the renderer-sidecar smoke presets now also assert that default-lane diagnostics stay in sync with the selected semantics lane, so `builtin_passthrough` and `wasm_v1` no longer only prove plugin attach; they also guard the default-lane candidate/source/rollout contract.
- the Mouse Companion WebUI now also mirrors those runtime lane-decision diagnostics in a dedicated `Runtime Diagnostics` block, along with `appearance_plugin_kind / appearance_plugin_appearance_semantics_mode / appearance_plugin_selection_reason`, so Windows bring-up can inspect the current default-lane and plugin state without opening raw `/api/state` JSON first.
- that same WebUI diagnostics block now also derives a short `Lane Verdict` summary (`stay on builtin`, `stay on builtin (wasm fallback)`, or `<lane> candidate pending manual confirmation`), so the runtime default-lane state is readable without manually combining the three machine-coded default-lane fields first.
- the latest VS2026 compile regression on this lane was still a seam-drift issue rather than a contract change: `Win32MouseCompanionRenderPluginHost.h` must include the manifest-contract types used by `selection.tuning`, appearance parsing now depends on `MouseFx/Utils/StringUtils.h`, metadata path UTF-16 conversion must stay on `Utf16ToUtf8`, and star adornment offsets currently remain zero-default instead of inventing new style-profile fields.
- Windows real preview dynamic persona separation is now a bit stronger too:
  - `dreamy` (`cream+moon`) biases `follow` toward lighter lift, softer forward pull, and gentler grounding
  - `agile` (`night+leaf`) biases `drag/follow` toward sharper lean, reach, and stance
  - `charming` (`strawberry+ribbon-bow`) biases `hold/click` toward rounder bounce, blush/highlight, and softer mouth/ear response

## Runtime Lanes
- Stable lane: `scaffold`.
- Progressive lane: `core` (`mfx_entry_posix_host`).
- Policy: new cross-platform capability lands in `core` first, then backports as needed.

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
- Windows blacklist routing root fix is active:
  - pointer-driven effect suppression now resolves the process at the current screen point first on Windows
  - trail synthetic-follow generation is limited to a short post-input smoothing window
- Cross-platform click ripple baseline is active:
  - Windows no longer ignores `EffectConfig.ripple`
  - default click contract is now shorter, smaller, center-clear, softer-glow, and single-ring
  - double-ring regression was removed again on both Windows and macOS

### Input Indicator
- macOS/Windows label and streak semantics are aligned (`L xN`, `W+ xN`).
- Indicator wasm dispatch has dedicated lanes and a budget floor.
- `/api/wasm/load-manifest` auto-infers surface and avoids cross-surface misload.
- `SetInputIndicatorConfig` syncs runtime host immediately after apply.
- Missing/stale indicator manifest falls back cleanly to native mode.

### Plugin Management / WebUI
- Unified top-level `Plugin Management` section is active.
- WebUI apply flow is backend-state-driven (`post-apply reconcile + refresh`).
- settings-page shell launch is now slightly more centralized too:
  - Windows `AppShellCore` and POSIX `PosixCoreAppShell` no longer each own a separate `WebSettingsServer` startup sequence
  - shared `WebSettingsLaunchCoordinator` now owns lazy create + rotate-token + start result for `WebSettingsServer`
  - platform shells still keep their own `settingsLauncher->OpenUrlUtf8(...)` step, so only lifecycle policy is shared while OS URL-opening remains platform-specific
- Sidebar order is fixed:
  - `General -> Mouse Companion -> Cursor Effects -> Input Indicator -> Automation Mapping -> Plugin Management`
- Shell top-bar layout regression fix is active:
  - stable three-column layout (`brand / center status / actions`)
  - center status expands from the title boundary
  - short status stays visually one-line aligned with actions
  - long status wraps without deforming the action row

### Mouse Companion
- Backend reset remains in effect; old skeleton runtime stays removed.
- Plugin-first landing route is active (`Phase0 -> Phase1 -> Phase2`).
- Shared `IPetVisualHost` + `PlatformPetVisualHost` abstraction is the stable cross-platform visual-host seam.

#### Windows Pet
- Windows pet is in `Phase1.5`: real transparent layered host window + visible placeholder backend are active.
- Windows renderer plugin staging is now explicit too:
  - `Win32MouseCompanionRenderPluginHost` is the current Windows-first seam for renderer-owned appearance/persona semantics
  - current implementation is still `builtin native`, not wasm yet
  - the host contract is intentionally narrow so Windows can finish first while macOS keeps its current interface stable
- Shared placement contract is active on Windows:
  - `relative`
  - `absolute`
  - legacy `fixed_bottom_left`
  - `strict / soft / free`
  - target-monitor resolution
- Current Windows placeholder is no longer just an embedded renderer; the active backend path is now:
  - `window -> backend factory/registry -> renderer input -> renderer runtime -> scene builder -> painter`
- Current Windows placeholder already consumes:
  - shared action semantics
  - `PetPoseFrame`
  - appearance/action-library asset lanes
  - lightweight readiness diagnostics
- Windows pet backend selection diagnostics are now active too:
  - `preferred_renderer_backend_source`
  - `preferred_renderer_backend`
  - `selected_renderer_backend`
  - `renderer_backend_selection_reason`
  - `renderer_backend_failure_reason`
  - `available_renderer_backends`
  - `unavailable_renderer_backends`
  - `renderer_backend_catalog`
  - `real_renderer_preview`
  - `renderer_runtime_*`
  - test mouse-companion route now also returns `renderer_runtime_before / renderer_runtime_after / renderer_runtime_delta`
  - test mouse-companion route now also supports test-friendly `wait_for_frame_ms` and `expect_frame_advance`, so Windows renderer proof can wait briefly for a new frame and report pass/fail in one response
- Renderer backend lifecycle seam is now explicit too:
  - backend selection no longer stops at constructor success
  - factory now treats `Start() / IsReady() / LastErrorReason()` as first-class fallback signals
  - placeholder backend is the current always-ready reference implementation
- Backend preference parsing is now a separate seam too:
  - factory no longer owns env-string normalization directly
  - current canonicalization accepts `default` as an alias of `auto`
  - preference source resolution now also routes through a dedicated registry; current built-ins are `env` and final `default`
  - explicit preference requests now travel through the same registry path too, instead of bypassing source resolution
  - Windows visual host now forwards an internal runtime-config preference request into window/backend selection before renderer creation
  - if that internal runtime-config backend preference changes after host start, the window now attempts an in-place backend reselection and preserves the current renderer if the replacement selection fails
  - mouse companion config/json now already has hidden persistence lanes for backend preference source/name, but WebUI does not expose them yet
  - hidden backend preference fields now also round-trip through settings state, apply-settings payloads, and runtime/test diagnostics
  - runtime/test diagnostics now also report whether the hidden config preference is the active resolved preference via `configured_renderer_backend_preference_effective` and `configured_renderer_backend_preference_status`
  - renderer registry/factory diagnostics now also distinguish currently unavailable backends from simply unselected ones, so future real-backend rollout can report machine/runtime gating reasons without changing the host contract again
  - a `real` backend now has a complete internal preview pipeline (`asset resources -> scene runtime -> scene builder -> painter -> render`), and the preview output is already a pose/action-aware stylized pet instead of a pure diagnostics card; default selection still keeps it unavailable behind a rollout gate, so current placeholder-first behavior stays unchanged
  - that preview path now also adds action-specific overlays inside `scene -> painter`, and those overlays now vary stroke/alpha emphasis per action intensity too, so `click / hold / scroll / follow / drag` are easier to distinguish visually during Windows bring-up instead of relying only on diagnostics fields
  - the preview face is now action-aware too: brows, mouth arc, and blush strength react to `click / hold / scroll / follow / drag`, so real-preview verification is not limited to silhouettes and motion marks
  - the preview posture is now action-aware too: body lift, head offset, shadow compression, tail lift, and limb cadence all vary per action, so Windows bring-up can distinguish states from whole-body rhythm rather than only overlays
  - the preview body/head/limb silhouette emphasis is now action-aware too: stroke weight and chest emphasis vary with action intensity so state separation is visible even before reading overlay glyphs
  - the preview glow/shadow/palette emphasis is now action-aware too: glow size, shadow/pedestal alpha, and accent presence now vary with action intensity so overall mood changes along with the active state
  - the preview action rhythm is now more state-specific too: click rebound, hold squeeze, scroll bob, drag pull, and follow gait all ride renderer-owned time phases instead of only changing static amplitudes
  - that action rhythm now also propagates through appendages more coherently: ears, tail, hands, and legs share the same renderer-owned gait/squeeze/pull phases instead of moving as mostly independent amplitude offsets
  - idle preview now also has a lightweight time-driven life rhythm via `poseSampleTickMs`: breathing, subtle hand float, ear cadence, shadow breathing, and tail sway keep the Windows preview from freezing into a static card when no action is active
  - real preview motion semantics are now split behind a dedicated `Win32MouseCompanionRealRendererMotionProfile` seam, so future visual tuning does not keep inflating `SceneBuilder`
  - real preview action overlay geometry is now split behind `Win32MouseCompanionRealRendererActionOverlayBuilder`, so overlay variants evolve independently from core body/head/limb layout
  - real preview face geometry is now split behind `Win32MouseCompanionRealRendererFaceBuilder`, so expression tuning stays isolated from both main body layout and overlay-specific assembly
  - real preview accessory/badge assembly is now split behind `Win32MouseCompanionRealRendererAdornmentBuilder`, so lane badges, pose badge, and accessory markers do not keep leaking back into the main scene builder
  - real preview color/material assignment is now split behind `Win32MouseCompanionRealRendererPaletteBuilder`, and renderer-owned color/theme tokens now also travel through `Win32MouseCompanionRealRendererPaletteProfile`, so skin/theme/status tuning no longer requires touching geometry assembly or builder-local literals
  - real preview appendage geometry is now split behind `Win32MouseCompanionRealRendererAppendageBuilder`, so ears, tail, hands, and legs evolve independently from the core body/head frame
  - real preview core frame geometry is now split behind `Win32MouseCompanionRealRendererFrameBuilder`, so body/head/shadow/pedestal layout stays isolated from appendages, palette, face, adornment, and overlays
  - real preview builders now share `Win32MouseCompanionRealRendererLayoutMetrics`, so body/head size conventions no longer travel as scattered bare float parameters across seams
  - real preview builder seams now also share `Win32MouseCompanionRealRendererStyleProfile`, and frame/palette/appendage/face/adornment/overlay builders are actively consuming it for ratio/scale defaults, expression anchors, accessory geometry offsets, and frame/palette tuning values instead of keeping those values duplicated in implementation bodies
  - `renderer_backend_catalog` is now the structured source of truth for backend inventory; `available/unavailable` arrays remain as lightweight compatibility views
  - the `real` backend now also publishes explicit unmet requirements through both `renderer_backend_catalog[*].unmet_requirements` and top-level `real_renderer_unmet_requirements`
  - `real_renderer_unmet_requirements` is now expected to be empty on current code; default unavailability is controlled by rollout gate `MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE`
  - runtime/test diagnostics now also publish a derived `real_renderer_preview` summary so Windows bring-up can verify rollout gate, selected backend, preview-active state, action lane, pose lane, and asset-lane readiness without reading renderer internals
  - the selected Windows renderer backend now also reports a backend-owned runtime snapshot (`renderer_runtime_*`), so diagnostics no longer have to infer preview state only from controller-side cached fields
  - backend-owned runtime diagnostics now also include render-proof fields (`frame_count`, `last_render_tick_ms`, `surface_width`, `surface_height`) so Windows bring-up can confirm that a test event really produced a new rendered frame
  - render-proof helpers are now extracted out of the mouse-companion test route, and Windows test API now also exposes a compact `/api/mouse-companion/test-render-proof` path for frame-advance validation without returning the full runtime payload
  - Windows test API now also exposes `/api/mouse-companion/test-render-proof-sweep`, so bring-up can check a compact status/click/hold/scroll/move/hold-end proof sequence in one response instead of replaying those calls by hand
  - a matching Git Bash helper now exists too: `tools/platform/manual/run-windows-mouse-companion-render-proof.sh`, so Windows bring-up can call single proof or sweep proof without manually rebuilding curl payloads
  - a native Windows helper now exists too: `tools/platform/manual/run-windows-mouse-companion-render-proof.cmd` backed by `run-windows-mouse-companion-render-proof.ps1`, so Windows bring-up can run proof/sweep gates directly from Command Prompt / PowerShell without depending on Git Bash
  - WebSettings now also writes `%APPDATA%\\MFCMouseEffect\\websettings_runtime_auto.json` when opened, and the native Windows proof helper auto-consumes that file when `BaseUrl/Token` are omitted, so Windows bring-up no longer requires manual copy/paste of the loopback URL/token on each machine
  - that runtime handoff file is now emitted through a standard-library writer inside `AppShellCore`, so Windows bring-up does not rely on another ad-hoc third-party JSON include path just to dump `url/base_url/token`
  - `AppShellCore` must consume the public `WebSettingsServer::Token()` accessor for that handoff path instead of touching the private `TokenCopy()` helper, otherwise VS2026 Windows builds fail at access-control stage before real-preview testing can even start
  - VS2026/MSVC build compatibility now also relies on Windows-specific env reads staying off raw `std::getenv`, and placeholder/diagnostics helpers staying in sync with the current runtime signatures/constants; recent compile regressions were caused by those seams drifting, not by renderer contract changes
  - the sweep proof route and Git Bash helper now also produce pass/fail summaries, so Windows bring-up can detect missed frame-advance expectations without manually counting per-event proof rows
  - both the single proof path and the sweep path now support optional `expected_backend` and `expect_preview_active` checks, so Windows bring-up can verify renderer selection, preview activation, and frame advance through the same gated proof contract
  - real preview palette emphasis is now action-themed too: overlay accent, glow, body/head tint, and accent fill all lean toward the current action color family instead of only changing amplitude
  - the Windows Git Bash bring-up helper now also exposes a `real-preview-smoke` preset, so the shortest real-preview gate no longer needs callers to repeat the sweep/backend/preview expectation arguments by hand
  - that same smoke preset now also prints a short human-readable expectation checklist before running, so manual Windows bring-up can sanity-check env/gate assumptions without reopening docs first
  - real preview face detail is now more alive too: pupils shift with facing/action focus and eye highlights vary by state, so `click / hold / drag / follow` read less like the same sticker with a different mouth arc
  - face readability now also includes whisker focus cues: whiskers spread more on stronger reactions and tilt slightly with drag/scroll direction, so state differences no longer rely only on mouth/brow/eye changes
  - body/head cohesion is now slightly stronger too: a light neck bridge now sits between head and torso, so the Windows real preview reads less like separate stickers and more like one creature
  - front/rear depth now reads a bit more clearly too: lightweight shoulder/hip patches now reinforce body mass and layering without turning the preview into a noisy shaded character
  - appendage/detail readability is now slightly stronger too: tail tip and paw-pad accents are now assembled behind the appendage/painter path, so small material cues read consistently without reopening host/runtime contracts
  - tail/body attachment also reads a bit cleaner now: a lightweight tail-root cuff now reinforces the tail entry point so the rear silhouette feels less detached without turning into a separate shading system
  - head/ear attachment also reads a bit cleaner now: lightweight ear-root cuffs now reinforce the ear entry points so the top silhouette feels less pasted-on without adding another explicit state lane
  - face shape now reads a bit cleaner too: lightweight cheek/jaw contours now reinforce the lower head silhouette, so expression no longer relies only on eyes/brows/mouth/blush
  - nose/mouth placement now reads a bit cleaner too: a lightweight muzzle pad now reinforces the front-face plane, so nose and mouth no longer sit directly on a perfectly flat head circle
  - upper-face structure now reads a bit cleaner too: a lightweight forehead pad now reinforces the brow/eye zone, so the top half of the face no longer depends only on brows and highlights
  - eye-zone side structure now reads a bit cleaner too: lightweight temple contours now reinforce the eye-socket area, so the face feels less like isolated eyes placed on a flat circle
  - eye-zone lower structure now reads a bit cleaner too: lightweight under-eye contours now reinforce the lower eye socket, so the face volume reads more continuous around the eyes
  - face centerline now reads a bit cleaner too: a lightweight nose bridge now reinforces the brow-to-muzzle connection, so the front face reads less like separate upper/lower pads
  - torso volume now reads a bit cleaner too: lightweight belly/flank contours now reinforce the body mass, so the preview feels less face-heavy and less like a single flat torso oval
  - torso centerline now reads a bit cleaner too: a lightweight sternum contour now reinforces the chest-to-belly transition, so the body no longer depends only on a single chest patch plus one lower-belly oval
  - upper torso now reads a bit cleaner too: a lightweight upper-torso contour now reinforces the shoulder-to-chest transition, so the torso volume feels more continuous from the top down
  - back mass now reads a bit cleaner too: lightweight back contours now reinforce the torso rear plane, so the body no longer reads only as front-facing patches layered on one oval
  - limb attachment now reads a bit cleaner too: lightweight hand/leg root cuffs now reinforce appendage entry points, so limbs feel less like rounded rectangles pasted directly onto the torso
  - limb silhouette now reads a bit cleaner too: lightweight hand/leg silhouette bridges now reinforce the outer transition into the torso, so attachment reads less like a stack of local patches
  - whole-pet proportion now reads a bit cleaner too: `follow / hold / click / drag` now lightly bias head scale, hand reach, and leg stance, so state changes register in the full silhouette instead of only in local detail layers
  - body stance now reads a bit cleaner too: `follow / hold / click / drag` now also lightly bias body width/height/center lift, so the torso block itself participates in the state change instead of remaining visually neutral
  - appendage proportion now reads a bit cleaner too: `follow / hold / click / scroll` now lightly bias tail width/height and ear spread/lift, so tail/ears participate in the same whole-pet state language instead of staying on default geometry
  - atmosphere grounding now reads a bit cleaner too: `follow / hold / drag` now lightly bias glow/shadow/pedestal scale, so the preview's ambient block and landing feel shift with the same state language instead of staying visually static under the pet
  - atmosphere grounding position now reads a bit cleaner too: `follow / hold / drag` now also lightly bias shadow/pedestal offset, so the landing feel changes in both size and placement instead of only as centered scaling
  - atmosphere grounding weight now reads a bit cleaner too: `follow / hold / drag` now also lightly bias shadow/pedestal alpha, so the preview feels lighter or more planted through opacity as well as geometry
  - upper atmosphere now reads a bit cleaner too: `follow / hold / drag` now also lightly bias glow alpha/offset, so the ambient cap above the pet moves with the same state language instead of staying visually fixed while grounding changes underneath it
  - click/scroll atmosphere now reads a bit cleaner too: those two states now also lightly bias glow/shadow/pedestal instead of leaving the atmosphere lane mainly to `follow / hold / drag`
  - atmosphere tint now reads a bit cleaner too: shadow/pedestal now also pick up a light action tint, so upper/lower atmosphere feel like one state language instead of separate color systems
  - ear silhouette stability now also depends on left/right swing staying mirrored inside `Win32MouseCompanionRealRendererAppendageBuilder`; if both ears consume the same horizontal swing sign, Windows real preview can regress into one oversized ear instead of the intended alert asymmetry
  - ear rendering stability now also depends on `Win32MouseCompanionRealRendererPainter` keeping ear fill on a bounded polygon path instead of a free overshooting closed-curve spline; otherwise a valid four-point ear can still render as a single oversized blob on one side
  - ear geometry stability now also depends on `Win32MouseCompanionRealRendererAppendageBuilder` clamping pose spread / swing / tip offset into a mirrored safe range; otherwise a valid runtime pulse can still push one ear far enough outward to read as oversized even after painter-side spline overshoot is removed
  - ear depth stability now also depends on front/rear ear scale staying explicit inside `Win32MouseCompanionRealRendererAppendageBuilder`; facing should keep one ear slightly recessed instead of letting both ears compete for the same visual area during motion
  - ear depth readability now also depends on rear-ear palette separation staying explicit across `Win32MouseCompanionRealRendererPaletteBuilder` and `Win32MouseCompanionRealRendererPainter`; front/rear ears should differ in alpha and inner-ear warmth so facing reads from color depth as well as scale
  - ear/head attachment readability now also depends on ear-root cuff depth staying explicit across palette/painter; the front ear root should stay brighter than the rear root so the head-top silhouette reads as one layered form instead of two equal pasted cuffs
  - ear outline depth now also depends on front/rear stroke separation staying explicit across palette/painter; the rear ear should draw with a softer, lighter outline than the front ear so facing remains readable even when fill colors get visually close under action tinting
  - inner-ear depth now also depends on front/rear inset geometry staying explicit inside `Win32MouseCompanionRealRendererPainter`; the rear inner-ear polygon should sit smaller/deeper than the front one so ear depth still reads when the outer ear silhouettes are close
  - rear-ear head-top depth now also depends on a small occlusion cap staying explicit across appendage geometry and painter layering; the rear ear should be lightly tucked under the head top instead of sitting at the same z-read level as the front ear
  - front/rear ear attachment geometry now also depends on cuff/cap size separation inside `Win32MouseCompanionRealRendererAppendageBuilder`; the front ear should keep a fuller root/cap silhouette while the rear ear stays slightly trimmed, otherwise both ears still read too similar even with depth tinting and occlusion in place
  - front/rear ear attachment staging now also depends on cuff/cap position separation inside `Win32MouseCompanionRealRendererAppendageBuilder`; the front ear should land slightly farther out/lower while the rear ear stays slightly tucked in/higher, otherwise both attachment clusters still read too centered even after size separation is added
  - front/rear ear sheet depth now also depends on outer-spread and tip-offset separation inside `Win32MouseCompanionRealRendererAppendageBuilder`; the front ear should open slightly wider and project its tip a bit farther than the rear ear so ear depth still reads even before fill/stroke differences are noticed
  - tail facing readability now also depends on root/tip projection separation inside `Win32MouseCompanionRealRendererAppendageBuilder`; the tail root should stay slightly tucked while the tip projects a bit farther along facing direction so whole-pet orientation is not carried only by the head/ears
  - tail depth readability now also depends on root/tip palette separation across `Win32MouseCompanionRealRendererPaletteBuilder` and `Win32MouseCompanionRealRendererPainter`; the tail root should stay slightly darker/recessed while the tip stays lighter/more present so tail direction reads even before geometry changes are noticed
  - tail continuity now also depends on a dedicated `tail mid contour` layer across `AppendageBuilder + PaletteBuilder + Painter`; tail depth should read as `root -> mid -> tip` instead of two isolated accents at the ends
  - tail mid-volume now also depends on a small tipward shape bias inside `Win32MouseCompanionRealRendererAppendageBuilder`; the mid contour should project slightly longer/flatter toward the tip so facing reads from the whole tail body instead of only root/tip markers
  - tail root-to-mid continuity now also depends on a lightweight `tail bridge` layer across `AppendageBuilder + Painter`; the renderer should not read like separate cuff/mid ellipses when tail depth is otherwise stable
  - tail mid-to-tip continuity now also depends on a lightweight `tail tip bridge` layer across `AppendageBuilder + Painter`; the tail should read as one continuous silhouette from root to tip instead of mid and tip competing as separate accents
  - head-top side cohesion now also depends on lightweight `parietal bridge` layers across `FaceBuilder + Painter`; crown and occipital should read as one continuous head shell near ear-top instead of a center pad plus detached side patches
  - head-top to ear-root cohesion now also depends on lightweight `ear skull bridge` layers across `FaceBuilder + Painter`; the head shell should visually catch the ear-root zone instead of leaving crown/parietal volume and ear cuffs to read as separate systems
  - head-to-body outer silhouette now also depends on lightweight `head shoulder bridge` layers across `FrameBuilder + Painter`; neck bridge alone should not carry the whole transition from head shell into upper torso
  - body-side silhouette cadence now also depends on lightweight `torso cadence bridge` layers across `FrameBuilder + Painter`; shoulder/back/flank patches should read as one continuous body rhythm instead of separate side accents
  - rear-body to tail-root cadence now also depends on lightweight `tail haunch bridge` layers across `FrameBuilder + Painter`; hind-quarter mass and tail-root staging should read as one continuous rear silhouette instead of unrelated flank and tail accents
  - facing depth now also depends on width separation across `parietal / ear-skull / tail-haunch` bridge layers; the front side should stay slightly fuller while the rear side stays slightly tighter so head shell, ear zone, and hind quarter all reinforce the same orientation cue
  - limb-entry silhouette cadence now also depends on lightweight `hand/leg cadence bridge` layers across `AppendageBuilder + Painter`; limb entry should read as a continuous body-to-limb transition instead of root cuff plus one isolated bridge patch
  - head-top cohesion now also depends on a light crown/occipital layer staying explicit across `Win32MouseCompanionRealRendererFaceBuilder` and `Win32MouseCompanionRealRendererPainter`; forehead and ear-top should read as one continuous head volume instead of separate front-face and ear clusters
  - appearance-profile parity now also depends on `Win32MouseCompanionRealRendererPaletteProfile` consuming more than a single skin base fill; Windows real preview now expands `skinVariantId + appearanceAccessoryIds` into a fuller body/head/ear/accessory/pedestal palette family so asset intent can influence the preview in a way that is closer to macOS model-theme feedback
  - appearance-profile parity now also depends on `Win32MouseCompanionRealRendererFrameBuilder + Win32MouseCompanionRealRendererFaceBuilder` consuming `skinVariantId` for lightweight structural bias; Windows real preview now lets `cream / night / strawberry` slightly bias body/head ratio and muzzle/jaw/cheek/forehead emphasis so appearance assets affect silhouette and facial structure, not only palette
  - appearance-profile parity now also depends on `Win32MouseCompanionRealRendererAppendageBuilder + Win32MouseCompanionRealRendererFrameBuilder` extending that same `skinVariantId` bias into ear scale, tail thickness, and shoulder/hip patch fullness; the family look should read across the whole pet instead of stopping at frame/face alone
  - appearance-profile parity now also depends on `Win32MouseCompanionRealRendererFaceBuilder` extending `skinVariantId` bias into expression tuning too; `cream / night / strawberry` now lightly bias brow tilt, mouth reactivity, pupil focus, highlight intensity, and whisker spread so the same action does not read as exactly the same face family across every skin
  - appearance-profile parity now also depends on `Win32MouseCompanionRealRendererAppendageBuilder` extending `skinVariantId` bias into body/appendage action cadence too; `cream / night / strawberry` now lightly bias follow reach, hold stance, ear spread, click ear-lift, and tail width/height response so movement character is no longer identical across all skins
  - appearance-profile parity now also depends on `Win32MouseCompanionRealRendererMotionProfile` extending `skinVariantId` bias into renderer-owned action synthesis; `cream / night / strawberry` now also lightly bias follow lift, click squash, drag lean/body-forward, hold head-nod, and follow/scroll tail response so full-pet motion character differs slightly by skin without introducing a new controller-visible action model
  - those `appearance profile` branches are now also centralized behind `Win32MouseCompanionRealRendererAppearanceSemantics`, so palette/frame/face/appendage/motion builders consume one shared renderer-owned semantics source instead of each re-expanding `cream / night / strawberry` locally
  - `appearance lane` now also carries combo semantics instead of only independent skin/accessory effects: accessory family (`moon / leaf / ribbon-bow / star`) can now lightly bias head/face/appendage/motion semantics through that same centralized source, so Windows preview starts consuming appearance combinations as one system instead of two unrelated lanes
  - that same combo semantics lane now also reaches preview mood/grounding too: accessory family may lightly bias glow/accent tint strength, shadow/pedestal weight, and action-overlay alpha emphasis through the same centralized source, so `skin + accessory` affects the whole preview atmosphere instead of only the pet silhouette
  - combo semantics are now also allowed to resolve stable renderer-owned persona presets (current examples: `cream+moon -> dreamy`, `night+leaf -> agile`, `strawberry+ribbon-bow -> charming`), so a few high-value `skin + accessory` pairs can bias frame/face/appendage/motion/mood together as one package instead of relying only on many unrelated local multipliers
  - accessory-profile parity now also depends on `Win32MouseCompanionRealRendererAdornmentBuilder + Win32MouseCompanionRealRendererPainter` preserving a small renderer-owned accessory family instead of collapsing every `appearanceAccessoryIds` entry into one star; Windows real preview now distinguishes at least `star / moon / leaf / ribbon-bow` adornment geometry so asset intent affects shape as well as color
  - accessory-profile parity now also depends on asset-family-specific attachment staging inside `Win32MouseCompanionRealRendererAdornmentBuilder`; moon/leaf/ribbon should not all land on the exact same head anchor if Windows preview is trying to approximate asset semantics rather than recolor one generic badge
  - accessory-profile parity now also depends on family-specific internal detail staying explicit across `Win32MouseCompanionRealRendererAdornmentBuilder + Win32MouseCompanionRealRendererPainter`; moon inset highlight, leaf vein, and ribbon fold lines should keep accessory families readable even when their overall bounds are similar
  - accessory-profile parity now also depends on family-specific motion staging inside `Win32MouseCompanionRealRendererAdornmentBuilder`; moon should feel lighter, leaf should sway more with drag/scroll direction, and ribbon should bounce/settle with click/hold so asset families differ in behavior as well as static shape
- Current boundary:
  - visible backend is stable enough for `Phase1.5` structural work
  - Windows still does not render the real 3D model yet
  - the main Win-vs-mac gap is now renderer form factor rather than missing appearance-lane plumbing: macOS already renders model assets through SceneKit, while Windows still approximates that asset intent through stylized 2D preview contracts
  - real-renderer work should plug in behind the existing backend/runtime seams instead of reopening host/controller layers
- Read on demand:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-phase1-plan.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-phase15-exit-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-real-renderer-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/ops/windows-mouse-companion-manual-checklist.md`

#### macOS Pet
- macOS Phase1 visual host is active:
  - model-first with placeholder fallback
  - `.usdz` preferred when SceneKit can load it
  - runtime action updates are forwarded (`idle / follow / click_react / drag / hold_react / scroll_react`)
- Shared placement contract is active on macOS:
  - `relative`
  - `absolute`
  - legacy aliases kept for compatibility
  - `strict / soft / free`
- Runtime `size_px` resize path is active and no longer create-time-only.
- Click/head-tint parity work is active:
  - click one-shot restarts immediately
  - tint fades continuously on frame ticks
  - click stays on a tauri-style press/rebound envelope
- Idle/follow parity direction is active:
  - `hover -> idle`
  - `follow` is defined as an upright walk, not crawl
  - drag-only yaw remains separate from idle/follow facing
- Scroll direction visual is active:
  - one scroll direction keeps the pet upright
  - the opposite direction flips presentation head-down without changing skeleton solving
- Current known boundary:
  - `.usdz` framing still needs care because projected bounds can under-report height on some paths
- Read on demand:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-plugin-landing-roadmap.zh-CN.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-action-clip-contract.zh-CN.md`

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
- Canonical regression entry:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- macOS daily shortcut:
  - `./mfx run-no-build` / `./mfx fast` skip both core and WebUI rebuilds
  - `./mfx run` / `./mfx start` perform fresh build preparation

## Packaging / Startup Truth

### Tray Menus
- macOS tray menu intentionally exposes only:
  - `Star Project`
  - `Settings`
  - `Exit`
- Windows tray menu now follows the same product rule.

### Launch At Startup
- macOS:
  - LaunchAgent now uses `tray` mode, not `background`
  - explicit settings toggle rewrites plist and applies `launchctl`
  - normal app startup repairs plist only and does not bootstrap/bootout service state
- Windows:
  - uses `HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run`
  - current executable path is rewritten idempotently so relocation can self-heal
  - detail doc:
    - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-launch-at-startup-contract.md`

### macOS Packaging
- Preferred entrypoint:
  - `./mfx package`
- Aliases:
  - `./mfx package-no-build`
  - `./mfx pack`
  - `./mfx pkg`
- `run/start/package` now share the same macOS core/WebUI preparation helper.
- Packaged output truth:
  - standard `MFCMouseEffect.app`
  - `Install/macos`
  - folder + `.zip` + unsigned `.dmg`
- Current package policy:
  - only minimal pet runtime assets are bundled
  - wasm demo plugin keeps only runtime files
  - packaged host binary is stripped in-bundle
  - package icon is low-weight generated `MFX`
  - DMG layout is install-oriented (`left app / right Applications`)
  - `Install/macos/` is git-ignored
  - Gatekeeper/notarization is still deferred
- Detail doc:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/ops/macos-portable-packaging.md`

### Windows Packaging
- User-facing packaging entry is also `./mfx package`.
- Windows installer remains Inno Setup based under the hood.
- `mfx.cmd` is the Windows wrapper for the shared `mfx` bash entry.
- Current policy:
  - no bundled `d3dcompiler_47.dll`
  - no installer-side startup task duplication
  - install directory stays runtime-focused

### Local Dev Sync
- Repository root now also carries a Syncthing-focused `.stignore` for source-only macOS/Windows development sync; build outputs, IDE caches, package outputs, dependency caches, and generated `docs/.ai` maps should stay local instead of bouncing between machines.

## Contracts That Must Not Drift
- Keep stdin JSON command compatibility.
- Keep current wasm ABI compatibility unless migration is explicitly approved.
- Keep host-owned bounded rendering strategy; no raw shader ownership without architecture approval.
- Keep docs synchronized in the same change set for every behavior or contract change.

## P2 Routing
- P2 index:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
- Mouse companion plugin roadmap:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-plugin-landing-roadmap.zh-CN.md`
- Server structure:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/server-structure.md`
- Regression workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
