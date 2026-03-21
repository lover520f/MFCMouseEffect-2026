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
