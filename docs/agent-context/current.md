# Agent Current Context (P1, 2026-03-19)

## Purpose
- This file is the compact execution truth for daily work.
- Keep only active contracts and operational facts.
- Move detailed history/design discussion to P2 docs.

## Scope and Priority
- Primary host: macOS.
- Delivery order: macOS first, Windows regression-free, Linux compile/contract-level.
- macOS stack rule: new capability modules are Swift-first; avoid expanding `.mm` for new large modules.
- Windows VS2026 project sync (2026-03-21): `MFCMouseEffect/MFCMouseEffect.vcxproj` is now aligned with the split automation/server/wasm source layout and passes `Release|x64` direct project build again. The top-level `MFCMouseEffect.slnx` still fails early at `ValidateSolutionConfiguration` with `0 errors`, so treat that as a separate solution-metadata issue rather than a C++ compile regression.

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
- Shell top-bar layout regression fixed on 2026-03-20: the header is now a stable three-column layout (`brand / center status / actions`) instead of a single-line strip. The middle status area now expands from the title column boundary, stays visually one-line/high-aligned with the action row for short text, and only grows taller when longer warning/status text wraps (`white-space: normal; overflow-wrap: anywhere`). The desktop action row remains explicitly locked to a single line (`flex-wrap: nowrap; min-width: max-content`) and only falls back to wrapping on the narrow-screen breakpoint.

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
  - pet placement and pet action semantics are now separate contracts: walking `follow` remains an action lane, while panel placement is moving to the shared `relative / absolute` contract aligned with `Input Indicator` (legacy `fixed_bottom_left / follow` values remain accepted during migration).
  - `button_down` no longer forces `drag`; drag now starts only after pointer travel while pressed.
  - click streak/head-tint runtime diagnostics update with decay.
  - click pulse/intensity tuned for tauri parity: base `0.84` + streak step `0.05` (test profile `0.92 + 0.06`).
  - visual motion profile is unified for `click/hold/scroll` with prod/test dual presets (`mouse_companion.use_test_profile` switch).
  - WebUI cleanup (2026-03-20): mouse companion user panel is now a single lightweight form with folded `Placement / Asset Paths / Advanced Motion` sections; dev-only `Probe` + `Runtime` tabs and their controller modules were removed from the user page, while hidden test-profile values are preserved on apply if they already exist in config. Chinese locale now also localizes the folded section summaries plus `Absolute Position / Absolute X / Absolute Y`, so the pet settings page no longer mixes obvious English leftovers in zh-CN mode.
- Phase1 macOS visual host is active (model-first with placeholder fallback):
  - Swift bridge: `Platform/macos/Pet/MacosMouseCompanionPhase1Bridge.swift`.
  - Enabled config creates native panel and now prefers `relative` + `absolute` placement modes, with legacy `fixed_bottom_left` + `follow` aliases kept for config compatibility.
  - `size_px` runtime resize regression fixed on 2026-03-20: the Swift host no longer treats companion size as create-time-only. `mfx_macos_mouse_companion_panel_configure_v1` now carries `sizePx`, macOS updates `targetPetSizePx` plus panel/canvas sizing immediately on apply, and loaded-model resize now re-runs `normalizeModelTransform() + fitCanvasToModel()`. Runtime size changes now first reset the panel canvas back to the new `size_px` baseline before re-fitting the loaded model, then do an immediate geometry-space refit (`projectedRenderableBounds` against live transforms, not stale `presentation` poses) plus two follow-up render-frame refit passes, so the model scale, outer canvas, and strict edge-clamp bounds all shrink/grow together instead of staying on the old oversized panel.
  - `absolute_x / absolute_y / target_monitor` are now shared with the `Input Indicator` positioning contract; macOS absolute coordinates use target-screen top-left semantics and are converted to Cocoa bottom-left window origins inside the Swift host.
  - `edge_clamp_mode` regression fixed on 2026-03-20: the Swift host now receives the clamp mode through `mfx_macos_mouse_companion_panel_configure_v1` and applies distinct placement behavior for `strict` (fully in desktop union, including the Dock/menu-bar occupied screen frame), `soft` (partial off-screen allowed while keeping visible area), and `free` (no desktop clamp). The pet panel now uses a dedicated `NSPanel` subclass that overrides `constrainFrameRect` so AppKit no longer silently snaps strict mode back to `visibleFrame`.
  - Visual host now tries `Assets/Pet3D/source/pet-main.*` first (`.usdz` preferred when SceneKit loads), then falls back to `phase1://placeholder/usagi`.
  - Visual host now loads `action_library_path` into a built-in clip sampler; `clickReact` is sampled from `pet-actions.json` (binary-search keyframe + quaternion slerp + bone_remap).
  - SceneKit model path now has its own 60fps frame driver; click one-shot is no longer tied to sparse input-event ticks.
  - Imported model animations are cleared on load to avoid built-in clip interference with click parity.
  - When SceneKit loads the `.usdz` fallback, anonymous skeleton nodes are hydrated from sibling `.glb` joint metadata so `head/chest/ear/arm/leg` mappings still resolve.
  - SceneKit framing work remains in-progress for Usagi full-body fit.
  - Current known regression (active): `projectedRenderableBounds` can under-report rendered height for the `.usdz` path, causing panel aspect to drift toward square and clipping ear/head-root/foot extents.
  - Load-time fit now avoids oversized multiplier compensation and runs once on the first rendered frame, preferring snapshot bounds with alpha filtering and falling back to projection bounds when snapshot is unavailable.
  - Runtime bounds snapshot no longer paints the visible `SCNView` magenta during measurement; size-fit sampling now uses the existing transparent background and alpha scan, removing the temporary red/purple flash while settings are applied.
  - Panel debug border is removed again; the pet canvas no longer renders a blue outline in daily use.
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
  - Idle/hover parity (active): native follow selection no longer stays latched in `follow` while stationary. `TickPetVisualFrame()` now mirrors tauri's `moveSpeed` selection model: pointer speed above threshold => `follow`; otherwise => `idle`, with exponential pointer-speed decay on frame ticks (`8.5/s` prod, `6.0/s` test; threshold `45px/s` prod, `30px/s` test).
  - Hover mapping now follows the roadmap contract (`hover -> idle`): `hover_start/hover_end` remain observable plugin inputs, but pet visual selection stays on the continuous `idle/follow` chooser instead of inventing a separate hover action.
  - Hover blacklist boundary (active): pet hover entry now starts before regular effect blacklist gates; blacklisted apps still block hover wasm/native effects, but pet idle/hover visual state continues to work.
  - macOS visual host now gives `idle` a visible hover-state boost: when hover is active, idle intensity increases subtle bob/ear/hand motion for both the placeholder view and the SceneKit model without touching click semantics.
  - SceneKit action-library sampling is no longer click-only: the macOS visual host now parses loop clips for `idle/follow` in addition to `clickReact`, and applies them on the real model before idle procedural augmentation.
  - Real-model idle ear parity (active): SceneKit now mirrors tauri's idle procedural layer on top of the `idle` clip by driving ear Z-wave and hand sway against cached rest transforms; this fixes the previous state where the real Usagi model stayed almost static in idle because only click clips were sampled. The ear/hand procedural amplitudes now use tauri-style fixed idle amplitudes (no extra attenuation by hover idle intensity), because the previous native path was visually under-driving ear motion even after `idle` clip sampling was restored.
  - Idle ear jitter regression fix (active): procedural idle ear wave now drives only the representative ear-root node for each side. Explicit semantic pose playback can still use the multi-segment ear chain, but replaying the same procedural delta on every ear segment compounded the motion and reintroduced visible jitter.
  - Pose binding for the real model is now semantic instead of exact-name-only: `left/right ear/hand/leg` bindings can resolve Blender-style bones like `DEF-Ear.L`, `DEF-Arm.L.*`, `DEF-Leg.L.*`, preventing idle procedural motion from disappearing just because the asset uses different exact bone names.
  - Idle ear jitter fix (active): semantic pose binding now selects a single representative bone for each logical binding (for example the root ear bone) instead of rotating the entire matched Blender export chain at once; this removes compounded idle ear motion while keeping blacklist-decoupled pet hover/idle routing unchanged.
  - Idle root-motion alignment (active): the real-model SceneKit path no longer adds extra idle root pitch/yaw wobble on top of the `idle` clip + ear procedural layer. Idle keeps light body translation bob, but root rotation now stays closer to tauri so ear motion is less likely to read as high-frequency jitter.
  - Pose write conflict guard (active): SceneKit now treats all-zero semantic pose packets as no-op updates (with one-time rest clear after non-zero), so idle ear procedural motion is no longer reset every frame by empty pose packets. `apply_pose` dispatch is also main-thread deterministic (main direct / non-main sync) to avoid async queue jitter between pose writes and frame updates.
  - Hold parity stage1 (active): native semantic hold pose now moves closer to tauri's `applyHoldProcedural` silhouette. SceneKit semantic pose keeps the accepted ear fold, but hand/leg shaping has been pushed further toward a cuddle squat: hand roots now use lower X over-push plus a very small forward offset, hand yaw has been reduced while hand Z curl was increased so the upper arm reads as "hug forward" instead of "twist backward", leg roots still translate slightly inward/upward, and leg curl stays closer to a tucked squat. Semantic pose binding still resolves short chains for `left/right_hand` and `left/right_leg` (ears remain 3-node chains), but child hand/leg bones no longer replay the full parent delta; secondary hand/leg chain nodes now receive attenuated rotation-only follow, which prevents the lower arm from amplifying hold into a behind-the-back pose. In the loaded-model path, hold also applies a light tauri-style `chest` squash (`x/z` expand, `y` compress) plus a small `head` counter-tilt, but semantic core nodes must be restored to rest before these per-frame adjustments; otherwise scale/rotation would accumulate frame-over-frame and blow the whole model up.
  - Hold parity stage2 (active): pet visual hold timing is now partially decoupled from the generic hold-effect timer. Pet visual selection enters `holdReact` on stable press (`130ms` prod / `90ms` test, stable speed `<=24px/s` prod / `<=30px/s` test), keeps a release buffer from `mouse_companion.release_hold_ms`, and suppresses hold immediately after scroll (`720ms`) so the pet no longer waits entirely for the shared `hold` lane before reacting.
  - Scroll parity stage1 (active): scroll visual playback now uses a short single-beat flap consumer instead of long one-shot packages. Each incoming `scroll` event adds one pending flap, the active flap can be shortened immediately when newer/faster input arrives, and the next flaps inherit the latest faster duration instead of waiting for several slow packages to drain first. Production/test flap windows are now `0.26s..0.10s` / `0.20s..0.08s`, the waveform is a single-stroke `sin(pi * t)` beat (not a multi-oscillation `sin(2pi * 1.8 * t)` envelope), and the real-model ear Z rotation stays strictly mirrored left/right.
  - Scroll direction visual (active): the mac visual host now treats `scrollReact` as a signed host-only intensity. One scroll direction keeps Usagi upright; the opposite direction temporarily flips the visible layer `180°` around its presentation center so the pet appears head-down while staying front-facing. This implementation is intentionally presentation-only (`sceneView` / placeholder draw transform) and does not modify model-node position or skeleton pose solving.
  - Follow parity stage1 (active): `follow` is currently defined as an upright cross-step walk, not crawl. The body/root should stay nearly fixed (no whole-body roll, no follow pulse scaling, and the real-model path no longer samples the loop `follow` clip at all), while the limbs do a two-beat opposite-limb gait: left leg forward pairs with right hand forward, right leg forward pairs with left hand forward. Runtime pulse scaling is now limited to reactive `hold/scroll` transitions only, so `follow <-> idle` threshold crossings no longer cause occasional enlarge/shrink flicker. Real-model root yaw is currently fixed front-facing for `idle/follow/click one-shot`; pointer yaw is reserved for explicit drag-only semantics so walking no longer makes the body look slanted.
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
- macOS daily shortcut contract:
  - `./mfx run-no-build` / `./mfx fast` now skip both core rebuild and WebUIWorkspace rebuild by forwarding `--skip-build --skip-webui-build` to the manual runner; use `./mfx run` / `./mfx start` when a fresh WebUI/core build is required.
- macOS tray menu simplification (active):
  - The macOS status-bar menu now intentionally exposes only three user-facing entries: `Star Project`, `Settings`, and `Exit`.
  - Theme switching, effect-type switching, and tray-side reload/config mutation entries were removed from the menu because they could freeze the tray/event loop and are better served by the Web settings UI.
- macOS launch-at-startup contract (active):
  - Launch-at-startup now registers the packaged executable in `tray` mode instead of `background` mode.
  - Reason: the POSIX background path enables stdin EOF auto-exit monitoring, which is suitable for piped/manual background runs but causes LaunchAgent-started processes to exit immediately under login-session startup.
  - Startup-time registration was further split into two paths:
    - explicit settings toggle -> rewrite plist and apply `launchctl` runtime state immediately
    - normal app startup -> rewrite plist only, without bootstrapping/booting out LaunchAgent
  - Reason: reapplying `launchctl` during every normal startup could spawn or unload duplicate launch services, which made manual launch and login launch interfere with each other.
  - Expected behavior is now:
    - manual launch repairs stale plist paths (for example from repo binary to `/Applications/...`) without spawning a second instance
    - login launch uses the already-written LaunchAgent and should surface the menu-bar `MFX` item while staying alive
- macOS portable packaging (active):
  - `./mfx package` is now the preferred user entrypoint for a full build + macOS `.app` package; `./mfx package-no-build` skips both core and WebUI rebuilds, and `./mfx pack` / `./mfx pkg` stay as compatibility aliases.
  - `run/start/package` now share the same macOS core/WebUI preparation helper (`mfx_manual_prepare_core_host_binary`) instead of carrying separate build paths; this keeps full-build vs skip-build behavior consistent across local run and packaging flows.
  - `tools/platform/package/build-macos-portable.sh` now assembles a standard `MFCMouseEffect.app` bundle that embeds `mfx_entry_posix_host`, `MFCMouseEffect/WebUI`, the minimal pet runtime asset set, and the sample wasm plugin under `Contents/MacOS/plugins/wasm`.
  - The packaged app launcher is now a native Mach-O wrapper (`Contents/MacOS/MFCMouseEffect`) instead of a shell script, so Finder/LaunchServices can start the bundle directly.
  - macOS package icon is active again with low size impact: packaging now generates a minimal `MFX` text iconset directly via AppKit/Swift at package time and writes `CFBundleIconFile=AppIcon`. The icon payload is about `210K` (`AppIcon.icns`), packaged artifacts are about `2.5M` (`.dmg`) / `2.6M` (`.zip`), the packaged host binary is stripped in-bundle (`strip -x`) to reduce payload size, and the prior `svg -> sips` CoreSVG warning noise is gone.
  - Packaged `.app` launches no longer auto-open Web settings; startup behavior now matches the normal tray app instead of the old debug-first browser flow.
  - macOS package size trim (active): packaging now copies only the runtime-used pet payload (`pet-main.usdz` + `pet-actions.json` + `pet-appearance.json` + `pet-effects.json`) instead of the whole `Assets` tree; `pet-main.glb` remains a repo/dev asset and is excluded from packaged artifacts.
  - wasm package trim (active): the bundled demo plugin now keeps only `plugin.json` + `effect.wasm`; development helpers (`effect.js`, `.d.ts`, `.wat`, `samples/`) are excluded from packaged artifacts.
  - The generated app launcher (`Contents/MacOS/MFCMouseEffect`) exports `MFX_WEBUI_DIR` / `MFX_SCAFFOLD_WEBUI_DIR` and launches from `Contents/Resources` so repo-style relative pet asset paths keep working inside the app bundle.
  - Default portable artifact naming now explicitly marks Apple Silicon scope: `MFCMouseEffect-macos-arm64-portable` folder + `.zip`.
  - Default macOS packaging output root is now `Install/macos` (instead of `dist/package/macos`) so packaged artifacts live under the same install-family location as Windows outputs and avoid `install`/`Install` ambiguity on case-insensitive macOS volumes.
  - `package` now also emits an unsigned `.dmg` alongside the folder and `.zip`, using the already-validated `.app` bundle as the sole source of truth and exposing a real Finder `Applications` alias for drag-to-install flow.
  - DMG presentation is now installation-oriented instead of raw Finder defaults: packaging mounts a writable staging image, creates a real Finder `Applications` alias, assigns a lightweight system `Applications` icon to that alias, pins icon positions to `left = MFCMouseEffect.app`, `right = Applications`, closes the temporary Finder layout window before finishing the package so `./mfx package` no longer leaves the staging DMG open on screen, and converts the final image with `UDBZ` compression for smaller distribution size.
  - Package cleanup (active): the generated folder/dmg no longer includes a top-level `README.txt`; delivered artifacts now keep only install-relevant payloads (`.app` and `Applications` link inside the dmg).
  - `Install/macos/` is now git-ignored so repeated local packaging does not pollute the worktree with generated macOS artifacts.
  - Unsigned package constraint remains active: Finder launch may still be gated by Gatekeeper until Developer ID signing/notarization is added later.
  - App-bundle runtime resolution is now bundle-relative: packaged `.app` resolves WebUI from `Contents/Resources/MFCMouseEffect/WebUI`, resolves pet runtime assets from `Contents/Resources/MFCMouseEffect/Assets/Pet3D/source`, and carries a generated `pet-main.joints.json` so the USDZ runtime can hydrate skeleton names without shipping the larger `pet-main.glb`.
  - App-bundle startup stabilization (active): packaged `.app` launch no longer auto-opens settings, `NSApplication.finishLaunching()` is forced before tray creation, tray startup now runs before degraded input-capture warnings, and macOS warning notifications are dispatched asynchronously so first-run permission prompts do not block tray initialization.
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
- macOS packaging workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/ops/macos-portable-packaging.md`

## Documentation Governance
- `current.md` is P1-only and must stay compact.
- Keep long history in P2 docs and archives, not in this file.
- Context artifacts:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-index.json`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-map.md`
