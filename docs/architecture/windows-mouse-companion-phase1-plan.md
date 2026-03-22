# Windows Mouse Companion Phase1 Plan

## Purpose
- Define the Windows `Mouse Companion` landing path before visible rendering work starts.
- Keep `AppController` on shared pet semantics and move platform visuals behind a small host contract.
- Make later upgrades (`placeholder -> sprite -> model`) additive instead of another controller rewrite.

## Problem Classification
- `Design behavior / capability gap`
- Short evidence:
  - Shared pet input/action/pose contracts already exist in `MouseCompanionPluginV1Types.h`.
  - `AppController` previously called macOS bridge functions directly, so Windows could not add a visual host without further platform branching in controller code.

## Phase1 Goal
- Deliver a Windows pet visual-host architecture that can consume the existing shared runtime snapshot.
- Do not promise full macOS visual parity in the first step.
- First implementation target is `host contract + Windows skeleton + controller decoupling`, then visible placeholder rendering.

## Shared Contract

### Input Surface
- `MouseCompanionPetRuntimeConfig`
- `MouseCompanionPetInputEvent`
- `MouseCompanionPetPoseFrame`
- `PetVisualHostUpdate`

### Host Lifecycle
- `Start(config)`
- `Configure(config)`
- `Show() / Hide()`
- `Shutdown()`

### Asset and Pose Surface
- `LoadModel(path)`
- `LoadActionLibrary(path)`
- `ConfigurePoseBinding(boneNames)`
- `MoveFollow(point)`
- `Update(update)`
- `ApplyPose(frame)`

## Layering Rule

### Shared Layer
- File: `MouseFx/Core/Control/IPetVisualHost.h`
- Responsibility:
  - define a minimal platform-neutral pet visual contract
  - expose only host-facing lifecycle, placement, action, and pose APIs
- Non-goal:
  - no Win32 window handles
  - no SceneKit concepts
  - no renderer-specific types

### Platform Factory Layer
- File: `Platform/PlatformPetVisualHost.h/.cpp`
- Responsibility:
  - create the platform visual-host implementation
  - keep platform branching out of `AppController`

### Controller Asset Coordination Layer
- Files:
  - `MouseFx/Core/Control/PetVisualAssetCoordinator.h/.cpp`
- Responsibility:
  - normalize repeated `model / action library / appearance profile` apply flow
  - return compact apply results back to `AppController`
  - keep controller code focused on path resolution + runtime status ownership

### Windows Platform Layer
- Files:
  - `Platform/windows/Pet/Win32MouseCompanionVisualState.h`
  - `Platform/windows/Pet/Win32MouseCompanionVisualHost.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPresenter.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionVisualRuntime.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionActionRuntime.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderScene.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderAccessory.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderAdornment.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderGait.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderExpression.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderPalette.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderRhythm.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionPlaceholderSilhouette.h/.cpp`
  - `Platform/windows/Pet/Win32MouseCompanionReactiveMotion.h/.cpp`
- Responsibility:
  - own Windows pet runtime state
  - split `Host / Presenter / Window / Renderer` responsibilities incrementally
  - keep the current first step small and compile-safe

## Controller Refactor Contract
- `AppController` must no longer store a raw macOS pet visual handle.
- `AppController` may know:
  - shared pet runtime config
  - shared action/pose semantics
  - one `IPetVisualHost`
- `AppController` must not know:
  - Swift bridge functions
  - Win32 window primitives
  - platform-specific pose-application details

## Windows Phase1 Scope

### Included Now
- shared host abstraction
- platform factory
- Windows host skeleton
- `AppController` decoupling from direct macOS bridge calls
- VS project integration

### Deferred to Next Step
- sprite/model rendering
- richer action animation parity beyond placeholder
- deeper pose-driven bone playback on Windows visual host
- final presenter split (`Window / Presenter / Renderer`) once placeholder behavior stabilizes

## Current Phase1.5 Status
- Windows now has a real transparent layered pet host window.
- Placeholder rendering is active through a dedicated renderer instead of being embedded in the host class.
- Placement already consumes shared config:
  - `relative`
  - `absolute`
  - legacy `fixed_bottom_left`
  - `strict / soft / free` clamp
  - target monitor resolution through `PlatformDisplayTopology`
- Startup anchor fallback is now defined: when Windows pet has not yet received a pointer sample, `relative` placement resolves against the target monitor center instead of `(0,0)`, so first-show placement is stable before the first move event.
- Current renderer is intentionally lightweight:
  - no model loading yet
  - no full pose-bone playback yet
  - action semantics are shown through placeholder silhouette changes for `idle / follow / click_react / drag / hold_react / scroll_react`
  - placeholder now separates action silhouettes more clearly:
    - `follow`: wider ear lift + alternating leg spread
    - `click_react`: squash/compress pulse
    - `drag`: wider paw spread
    - `hold_react`: deeper squat + short eyes
    - `scroll_react`: signed body lean
  - placeholder now also consumes shared runtime payload instead of only action names:
    - latest `PetPoseFrame` samples are mapped onto placeholder ear/hand/leg offsets
    - asset readiness hints (`model path present`, `action library path present`, `pose binding/pose samples present`) are surfaced as lightweight status badges
    - `pet-actions.json` is now parsed on Windows too; a small non-GPU action sampler feeds body/head squash/lean from clip tracks into the placeholder renderer
    - `pet-appearance.json` is now a first-class host asset path on Windows; appearance reload no longer has to piggyback on a full `TryLoadDefaultPetModel()` cycle
    - controller-side asset apply flow is being normalized: `model / action library / appearance profile` are now treated as separate visual-host asset lanes instead of one monolithic reload path
    - `model` lane now also owns its own runtime-status update path (`loaded_model_path / loaded_model_source_format / model_load_error`) instead of relying only on the aggregate initial-load branch
    - window geometry and clamp resolution have started moving out of `Win32MouseCompanionWindow` into a dedicated presenter helper, so display-space policy is no longer tied directly to layered-window plumbing
    - host-side `state_` mutation is also being thinned: follow/action/pose/application now goes through a small visual-runtime helper instead of hand-written field assignment blocks in `Win32MouseCompanionVisualHost`
    - action clip timeline state is now moving out of the host too: active action key, one-shot restart logic, and sampled clip refresh are handled by a dedicated action-runtime helper instead of staying embedded in the host class
    - placeholder pre-render interpretation is now being separated from paint code: the scene helper converts `action/pose/appearance` state into draw-ready geometry/colors so the renderer itself keeps trending toward pure drawing
    - placeholder now also carries a lightweight facing contract: horizontal movement updates a left/right facing hint, and the current implementation uses a small accumulated-motion hysteresis so rapid micro-jitter does not constantly flip the facing side
    - placeholder procedural motion is now split from scene building: a dedicated motion helper drives idle/follow bob, ear sway, paw cadence, tail swing, blink, and shadow squash before the scene builder converts that into draw-ready geometry
    - reactive pulses for `click_react / hold_react / scroll_react / drag` are now split once more into a dedicated helper, so placeholder motion consumes a small time-decayed reaction contract instead of only branching on raw action labels
    - placeholder face/chest details are now also reaction-aware: whiskers, blush, chest bob, glow, and mouth openness all consume the same reactive timeline instead of adding more renderer-local branching
    - facial expression assembly is now split again from both scene layout and paint code: pupils, brows, and mouth-arc semantics are produced by a dedicated expression helper so renderer responsibility keeps narrowing
    - placeholder gait support is now split too: neck/head-body bridge, limb bridge lines, and back/belly/hip shape accents are assembled separately so the body no longer reads as disconnected ellipses
    - accessory resolution is no longer a single hardcoded oval in the renderer; a dedicated helper can now map appearance-profile accessory ids into simple bow/star-like head accessories without inflating the draw path further
    - placeholder palette/material resolution is now split from scene layout too: skin variant, tint, scroll accent, tail tip, and paw-pad colors are resolved once behind a dedicated palette helper before geometry assembly
    - placeholder posture baselines are now split too: `idle / follow / drag / hold / scroll` no longer rely only on additive per-part tweaks, because a dedicated posture helper now establishes base body/head stance, ear spread, fore/rear stance width, and shadow/body anchor bias before rhythm and pose offsets are applied
    - placeholder action-shape profiles are now split too: `click / hold / scroll / drag / follow` can reshape chest lift/width, tail root/curl, ear-root presence, shoulder/hip patch scale, and front/rear depth bias without inflating the main scene builder further
    - cross-part stride rhythm is now split too: ear/tail/fore-leg/rear-leg phase relationships are assembled behind a dedicated rhythm helper so silhouette timing no longer has to stay embedded in scene geometry math
    - explicit silhouette nodes are now split too: ear roots, tail root, shoulder/hip patches, and front/rear depth patches are assembled behind a dedicated helper so body-readability tweaks no longer bloat the main scene builder
    - accessory/motion accents are now split too: collar/charm and low-cost dust/motion cues are built behind a dedicated adornment helper instead of inflating scene layout or renderer branches further
  - boundary remains explicit: Windows still does not render the real 3D model yet, but the host contract now already carries the same data classes that the later real renderer will consume.

## Phase1.5 Closure And Next-Phase Routing
- Phase1.5 exit is now explicitly defined in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-phase15-exit-contract.md`
- Future real-renderer backend preparation is now explicitly defined in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-real-renderer-contract.md`
- First renderer-adapter seam is now active:
  - `Win32MouseCompanionWindow` no longer owns the placeholder renderer by concrete type only
  - the current default backend is resolved through `IWin32MouseCompanionRendererBackend` + `Win32MouseCompanionRendererBackendFactory`
  - this keeps the current placeholder backend intact while opening a single swap point for later renderer growth
- Renderer input handoff has also started:
  - renderer-facing payload now has an explicit `Win32MouseCompanionRendererInput`
  - `Win32MouseCompanionRendererInputBuilder` now isolates backend input from host/window-only state
  - this keeps placement/lifecycle policy out of later renderer implementations
- Placeholder backend/painter split has now started too:
  - `Win32MouseCompanionPlaceholderRenderer` is now only the placeholder backend coordinator
  - `Win32MouseCompanionPlaceholderSceneBuilder` owns placeholder scene assembly
  - `Win32MouseCompanionPlaceholderPainter` owns GDI+ painting only
  - built-in backend resolution no longer ends at one hardcoded constructor: `Win32MouseCompanionRendererBackendRegistry` orders registered backends by priority, while the placeholder backend explicitly self-registers during default-factory setup
  - this keeps future backend replacement from inheriting one large combined render/state file
- Renderer runtime interpretation is now narrowing too:
  - `Win32MouseCompanionRendererRuntime` centralizes action decoding, normalized intensities, pose-sample lookup, facing sign, and clip access
  - placeholder motion/posture/action-profile/scene helpers now consume that runtime view instead of repeatedly decoding raw renderer input on their own
- Backend-selection observability is now active too:
  - Windows visual host now reports `preferred_renderer_backend_source`
  - Windows visual host now reports `preferred_renderer_backend`
  - Windows visual host can now report `selected_renderer_backend`
  - runtime/test diagnostics also expose `renderer_backend_selection_reason` and `renderer_backend_failure_reason`
  - runtime/test diagnostics also expose `available_renderer_backends`
  - runtime/test diagnostics also expose `unavailable_renderer_backends`
  - runtime/test diagnostics now also expose `renderer_backend_catalog`
  - runtime/test diagnostics now also expose `real_renderer_preview`
  - runtime/test diagnostics now also expose backend-owned `renderer_runtime_*` fields from the currently selected renderer
  - runtime/test diagnostics now also expose `configured_renderer_backend_preference_effective` and `configured_renderer_backend_preference_status` so config/env/default precedence can be verified directly
  - backend preference updates that arrive through runtime config now trigger an in-place window/backend reselection instead of being ignored after the first backend is created
  - a `real` backend now has a complete internal preview pipeline through `Win32MouseCompanionRealRendererAssetResources`, `Win32MouseCompanionRealRendererSceneRuntime`, `Win32MouseCompanionRealRendererSceneBuilder`, and `Win32MouseCompanionRealRendererPainter`
  - that preview path is no longer just a readiness card; it now renders a stylized pet-like scene that reacts to action/facing/pose lanes under the hidden rollout gate
  - the preview path now also keeps action-specific differentiation inside the renderer seam itself: `click` adds a ring, `hold` adds a grip band, `scroll` adds orbit arcs, and `follow/drag` add motion overlays, and those overlays now also vary stroke/alpha emphasis with runtime intensity so visual bring-up can verify action changes without reading only JSON diagnostics
  - the same preview seam now also owns action-aware face semantics, so brow/mouth/blush changes stay renderer-local instead of expanding controller-side preview rules
  - whole-body posture is now also renderer-owned: center-of-mass lift, head offset, tail lift, shadow scale, and limb placement vary per action so visible state separation does not depend only on overlay glyphs
  - body/head/limb silhouette emphasis is now also renderer-owned: stroke weight and chest emphasis vary with action intensity so visible state separation does not depend only on overlay glyphs
  - glow/shadow/palette emphasis is now also renderer-owned: glow size plus shadow/pedestal/accent alpha vary with action intensity so overall mood changes do not require controller-side state styling
  - action rhythm is now also more renderer-owned and state-specific: click rebound, hold squeeze, scroll bob, drag pull, and follow gait now ride renderer-local time phases instead of only reusing static action amplitudes
  - appendage coordination is now also more rhythm-aware: ears, tail, hands, and legs now reuse those renderer-local phases so follow/hold/scroll/drag feel more like one coordinated body instead of a set of unrelated offsets
  - idle life rhythm is now also renderer-owned and time-driven from existing runtime ticks, so breathing/ear cadence/tail sway stay inside the preview seam instead of introducing another controller-side idle animation track
  - preview motion tuning now has its own `Win32MouseCompanionRealRendererMotionProfile` seam, so action-strength curves can evolve without turning `SceneBuilder` into another multi-hundred-line behavior bucket
  - preview action overlay geometry now has its own `Win32MouseCompanionRealRendererActionOverlayBuilder` seam, so `click / hold / scroll / drag / follow` overlay iteration does not bloat the main scene builder again
  - preview face geometry now has its own `Win32MouseCompanionRealRendererFaceBuilder` seam, so brow/eye/mouth/blush tuning can evolve independently from both body layout and overlay assembly
  - preview accessory/badge assembly now has its own `Win32MouseCompanionRealRendererAdornmentBuilder` seam, so lane badges, pose badge, and accessory markers evolve independently from both posture/layout and overlay geometry
  - preview color/material assignment now has its own `Win32MouseCompanionRealRendererPaletteBuilder` seam, and its renderer-owned theme tokens now also travel through `Win32MouseCompanionRealRendererPaletteProfile`, so skin/theme/status color iteration does not re-couple geometry and presentation again
  - preview appendage geometry now has its own `Win32MouseCompanionRealRendererAppendageBuilder` seam, so ears/tail/hands/legs can evolve without pushing more pose math back into the core body/head scene builder
  - preview core frame geometry now has its own `Win32MouseCompanionRealRendererFrameBuilder` seam, so the remaining body/head/shadow/pedestal layout no longer keeps the top-level scene builder as a catch-all
  - preview builders now share `Win32MouseCompanionRealRendererLayoutMetrics`, so body/head sizing rules are centralized instead of being re-passed as loose float parameters between builder seams
  - preview builders now also share `Win32MouseCompanionRealRendererStyleProfile`, and the active frame/palette/appendage/face/adornment/overlay seams now consume it for ratio/scale defaults, face-expression anchors, accessory geometry offsets, and frame/palette tuning values so visual tuning no longer requires editing the same constants in multiple files
  - default diagnostics should now show `real` as `unavailable(rollout_disabled)` instead of `requirements_unmet`
  - current `real_renderer_unmet_requirements` should be empty; rollout is now controlled by the hidden test gate `MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE=1`
  - `real_renderer_preview` now acts as the current bring-up truth view for rollout gate state, preview-active state, current action lane, pose lane, and asset-lane readiness
  - backend-owned runtime diagnostics now travel through `renderer -> window -> visual host -> AppController -> settings/test diagnostics`, so future renderer swaps do not need a second controller-side inference path
  - those runtime diagnostics now also include render-proof counters/timestamps/surface-size fields, making it possible to verify that a dispatched test event caused a fresh frame instead of only changing logical state
  - the test mouse-companion route now returns explicit `renderer_runtime_before / after / delta` payloads, so renderer proof no longer requires two manual snapshot requests and client-side diffing
  - the same route now also supports bounded waiting (`wait_for_frame_ms`) and expectation reporting (`expect_frame_advance`), which keeps Windows renderer verification test-friendly without changing production runtime behavior
  - compact render-proof handling is now split into its own helper seam, and `/api/mouse-companion/test-render-proof` now exists as a minimal verification path when we only need renderer frame proof and preview summary rather than the full mouse-companion runtime object
  - `/api/mouse-companion/test-render-proof-sweep` now exists as the compact bring-up path when we want one response covering `status / click / hold_start / scroll / move / hold_end` proof transitions instead of manually replaying multiple proof calls
  - a matching Git Bash helper now exists too: `tools/platform/manual/run-windows-mouse-companion-render-proof.sh`, so Windows bring-up can hit either the single proof or sweep proof route without rebuilding curl payloads by hand
  - the sweep proof route/helper now also emit compact pass/fail summaries, so bring-up can fail fast when frame-advance expectations are missed instead of forcing manual inspection of every row
  - the compact proof path and the sweep proof path now both support optional expected-backend and preview-active checks, so bring-up can validate renderer selection, preview activation, and frame advance through the same expectation model
  - real preview palette emphasis is now action-themed too, so glow/body/head/accent colors shift slightly toward the current action family rather than relying on overlay geometry alone
  - the Windows Git Bash bring-up helper now also exposes a `real-preview-smoke` preset, keeping the shortest real-preview gate as a named preset instead of another repeated long command form
  - face detailing is now action-aware too: pupil focus and eye-highlight intensity are renderer-owned, so state readability no longer depends only on brows/mouth/overlay geometry
- Backend lifecycle fallback is now part of the seam:
  - registry/factory selection no longer treats constructor success as enough
  - backend startup now has an explicit `Start() / Shutdown() / IsReady() / LastErrorReason()` contract
  - this keeps future real-backend bring-up failures inside backend selection instead of leaking into host/window logic
- Backend preference resolution is now isolated too:
  - factory no longer owns environment-string normalization directly
  - current canonicalization keeps `default` as an alias of `auto`
  - future config-backed preference sources can reuse the same normalization seam without reworking backend selection
  - preference source routing is now behind a dedicated registry, so future `settings` / debug / env sources can layer by priority instead of expanding factory conditionals
  - explicit test/debug preference requests now also reuse that same source-resolution path instead of forming a parallel branch
  - internal runtime-config request forwarding now reaches the window before backend creation, so a future settings-backed preference no longer requires another window lifecycle rewrite
  - hidden config/json persistence lanes now exist for backend preference source/name, but they are intentionally not exposed in current settings UI
  - those hidden fields now also survive settings-state export, apply-settings ingestion, and runtime/test diagnostics without needing another contract expansion
- Intended next-step order:
  1. close Phase1.5 behavior/structure boundary
  2. keep the current placeholder as a stable backend
  3. start a renderer-facing adapter contract before choosing a final Windows rendering technology

## Why This Structure
- Prevents another controller rewrite when Windows pet becomes visible.
- Keeps platform growth open:
  - `Win32MouseCompanionVisualHost`
  - future `Win32MouseCompanionWindow`
  - future `Win32MouseCompanionPresenter`
  - future renderer/backend choices
- Matches repository rules:
  - small files
  - single responsibility
  - shared contract before platform detail

## Validation Gates
- `Release|x64` Windows project build must stay green.
- Existing macOS bridge path must remain reachable through the new host interface.
- Current runtime diagnostics must keep reporting pet runtime/visual-host fields without controller-side platform branching growth.

## Next Implementation Order
1. Add a real Windows transparent pet window shell.
2. Move placement/clamp logic into Windows presenter/state helpers.
3. Add visible placeholder rendering for `idle/follow/click/hold/scroll`.
4. Keep consuming the same shared plugin snapshot and pose frame.
