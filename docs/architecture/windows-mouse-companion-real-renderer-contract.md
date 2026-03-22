# Windows Mouse Companion Real Renderer Contract

## Purpose
- Define the interface boundary for the future Windows `Mouse Companion` real renderer.
- Ensure the current placeholder path remains a replaceable renderer backend instead of becoming a permanent special case.
- Keep renderer choice open while locking the data contract early.

## Problem Classification
- `Design behavior / future renderer interface contract`
- Short evidence:
  - Windows Phase1.5 already consumes shared inputs such as `model`, `action_library`, `appearance_profile`, `PetPoseFrame`, and runtime action state.
  - What is still missing is a renderer-facing contract that states how a future richer Windows backend plugs in without rewriting `AppController` or replacing the host lifecycle again.

## Non-Goal
This contract does **not** choose the final Windows rendering technology yet.
Examples intentionally left open:
- Direct2D
- DirectComposition
- Direct3D
- sprite-based hybrid renderer
- CPU model preview path

The goal is interface stability first, backend choice later.

## Renderer Position In The Stack
Expected long-term stack:
- shared controller/runtime
- platform visual host
- presenter/runtime coordinator
- renderer backend

Conceptually:
- `asset coordinator -> presenter/runtime -> renderer backend`

## Stable Upstream Inputs
A future Windows real renderer must be able to consume the same conceptual inputs already carried by the current host contract.

### 1. Model Lane
Input meaning:
- resolved model asset path
- resolved source format
- load success/failure diagnostics

Current source examples:
- `pet-main.usdz`
- future Windows-native converted/runtime-ready asset forms

Contract rule:
- renderer backend may choose its own runtime representation, but upstream lanes must still describe one canonical resolved model asset input

### 2. Action-Library Lane
Input meaning:
- action library path
- action clip selection state
- sampled action runtime state

Contract rule:
- action selection/sampling should remain outside the renderer where possible
- renderer should consume either sampled pose/output or compact clip-selection/runtime state, not own controller semantics

### 3. Appearance-Profile Lane
Input meaning:
- appearance profile path
- resolved color/material/accessory semantics
- load success/failure diagnostics

Contract rule:
- appearance profile remains a first-class asset lane independent of full model reload
- renderer consumes resolved appearance intent, not controller-specific branching

### 4. Pose-Frame Lane
Input meaning:
- latest `PetPoseFrame`
- per-bone semantic transforms / pose sample data
- optional pose binding state

Contract rule:
- pose transport stays shared and platform-neutral
- renderer may bind it to real bones, sprites, rig handles, or future adapters, but the upstream `PetPoseFrame` lane must not fork into a Windows-only schema

### 5. Runtime Action Lane
Input meaning:
- current semantic action (`idle / follow / click_react / drag / hold_react / scroll_react`)
- direction/facing hints
- action intensity / transient pulse state
- placement/follow anchors already normalized by presenter/runtime layers

Contract rule:
- renderer consumes action state as presentation intent
- controller and host keep ownership of action semantics and placement policy

## Required Downstream Responsibilities
A future real renderer backend is expected to own only renderer-local concerns:
- visual resource creation/destruction
- scene/mesh/sprite/model instance ownership
- per-frame draw/update application
- renderer-local material/shader/scene graph details
- renderer-local animation interpolation if needed

It should **not** own:
- top-level placement policy
- monitor/clamp rules
- controller config parsing
- launch/show/hide lifecycle policy
- cross-platform runtime semantics

## Recommended Internal Interface Split
The future Windows real renderer can be implemented behind three narrow concepts.

### Asset Loader / Resource Adapter
Responsibility:
- translate resolved model/action/appearance inputs into backend resources
- cache/reload backend resources
- report load failures back to host/runtime diagnostics

### Scene Runtime Adapter
Responsibility:
- receive runtime action + pose + facing inputs
- map them onto backend scene state
- keep renderer-frame state independent from host lifecycle glue

### Renderer Backend
Responsibility:
- perform final visual update/draw
- manage backend-specific device/context/swapchain/composition details

## Compatibility Rule With Placeholder
The current placeholder path should remain a valid backend implementation of the same high-level contract.

This means:
- placeholder and future real renderer should both be able to sit behind the same host/presenter/runtime flow
- switching renderer backend should not require changing `AppController` semantics
- switching renderer backend should not require redefining model/action/appearance/pose inputs

## Exit Criteria For Starting Real Renderer Work
Real renderer implementation work should begin only when all of the following are true:
1. Phase1.5 placeholder has been declared stable enough by `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-phase15-exit-contract.md`
2. shared input lanes are already observable through diagnostics
3. presenter/runtime layers are no longer growing because of placeholder-specific draw concerns
4. there is a clear decision whether the next milestone targets:
   - higher-quality 2D replacement, or
   - true model-driven renderer parity

## First Implementation Recommendation
Before touching GPU-specific code, the safest first step is:
1. add a renderer-facing adapter interface under `Platform/windows/Pet`
2. make the current placeholder renderer conform to that interface
3. prove renderer swap does not change host/controller contracts
4. only then add a second backend implementation

## Current Progress
- The first adapter seam is now active:
  - `IWin32MouseCompanionRendererBackend`
  - `Win32MouseCompanionRendererBackendFactory`
- Built-in backend routing no longer ends at one hardcoded constructor:
  - `Win32MouseCompanionRendererBackendRegistry`
  - placeholder backend explicit registration during default-factory setup
- `Win32MouseCompanionWindow` now depends on the renderer interface/factory seam instead of directly owning `Win32MouseCompanionPlaceholderRenderer` as a concrete-only dependency.
- A renderer-facing input seam is now active too:
  - `Win32MouseCompanionRendererInput`
  - `Win32MouseCompanionRendererInputBuilder`
- The current placeholder motion/posture/action-profile/scene path now trends toward renderer-input consumption instead of reading the whole host visual state directly.
- The placeholder backend has also split its pure paint path out:
  - `Win32MouseCompanionPlaceholderRenderer` is now the backend coordinator
  - `Win32MouseCompanionPlaceholderSceneBuilder` owns placeholder scene assembly
  - `Win32MouseCompanionPlaceholderPainter` owns GDI+ drawing only
- Runtime interpretation is now moving behind a reusable adapter too:
  - `Win32MouseCompanionRendererRuntime`
  - placeholder helper layers now consume a normalized runtime view instead of repeatedly decoding raw renderer input
- Backend selection is now observable through runtime diagnostics:
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
  - `configured_renderer_backend_preference_effective`
  - `configured_renderer_backend_preference_status`
  - this gives future backend bring-up a stable way to verify host/factory/registry routing before any visual parity work starts
  - `unavailable_renderer_backends` is reserved for registered-but-currently-unavailable backends so future real renderers can publish runtime gating reasons without faking constructor/start failures
  - the first concrete use of that lane now exists: backend name `real` is registered with a complete internal preview pipeline, but default availability is still gated as `rollout_disabled` so factory/diagnostics behavior can be exercised before wider rollout
  - `renderer_backend_catalog` is the structured backend inventory contract; future renderer rollout should extend it instead of adding more ad-hoc string lists
  - `real_renderer_preview` is the rollout-facing preview summary contract; it should remain stable enough for manual verification and AI-IDE consumption without requiring direct access to renderer-private scene objects
  - `renderer_runtime_*` is the backend-owned runtime snapshot contract; it should reflect the last successful renderer input/render state instead of forcing diagnostics to reconstruct preview state only from higher controller layers
  - `renderer_runtime_frame_count`, `renderer_runtime_last_render_tick_ms`, and renderer surface-size fields are part of the render-proof subset of that contract; future real-backend rollout should preserve their semantics so test automation can detect actual rendered-frame advancement
  - the current test route contract now includes `renderer_runtime_before / renderer_runtime_after / renderer_runtime_delta`; future renderer bring-up should preserve this diff-friendly proof shape instead of forcing callers to reconstruct transitions manually
  - the test route also accepts bounded wait/expect parameters (`wait_for_frame_ms`, `expect_frame_advance`) so proof-of-render can tolerate short asynchronous frame delays while staying explicit and machine-readable
  - a compact `/api/mouse-companion/test-render-proof` route now exists beside dispatch testing; it should remain focused on renderer proof + preview summary, and it now shares the same optional backend/preview expectation checks as the sweep path while `/api/mouse-companion/test-dispatch` remains the heavier end-to-end dispatch contract
  - a compact `/api/mouse-companion/test-render-proof-sweep` route now also exists for bring-up sequencing; it should remain focused on a small fixed proof sequence and reuse the same render-proof/result semantics instead of inventing a second diagnostics schema
  - Windows bring-up now also has a native `.cmd -> .ps1` entrypoint beside the Git Bash helper; future proof-route changes should keep both frontends aligned so Windows testing does not depend on Git Bash availability
  - WebSettings runtime handoff is now a machine-readable contract too: when the app opens WebSettings it writes `%APPDATA%\\MFCMouseEffect\\websettings_runtime_auto.json` with `url/base_url/token/port`, and the Windows-native proof helper should keep consuming that file so Windows bring-up does not depend on manual URL/token copy steps
  - preview palette emphasis is renderer-owned too; future visual tuning should keep action-family tinting behind palette/style contracts instead of scattering ad-hoc color shifts through painter code
  - face focus detailing is renderer-owned too; pupil offset and eye-highlight intensity should remain face-builder inputs derived from runtime motion state rather than controller-side flags or painter-local heuristics
  - whisker focus detailing is renderer-owned too; whisker spread/tilt should remain face-builder outputs derived from runtime motion state rather than another controller-visible action-specific toggle list
  - lightweight body/head cohesion cues such as a neck bridge should remain frame/painter-owned preview details, not a reason to reopen host/runtime contracts or add controller-side geometry flags
  - lightweight front/rear depth cues such as shoulder/hip patches should also remain frame/painter-owned preview details, so silhouette readability improves without creating another controller-visible detail state surface
  - lightweight tail-tip and paw-pad accents should remain appendage/painter-owned preview details too, so material/detail readability improves without creating another controller-visible accent-state surface
  - lightweight tail-root attachment cues such as a rear cuff should remain appendage/painter-owned preview details too, so tail/body cohesion improves without creating another controller-visible attachment-state surface
  - lightweight ear-root attachment cues such as top cuffs should remain appendage/painter-owned preview details too, so ear/head cohesion improves without creating another controller-visible attachment-state surface
  - lightweight cheek/jaw contour cues should remain face-builder/painter-owned preview details too, so lower-head silhouette readability improves without creating another controller-visible contour-state surface
  - lightweight muzzle-pad cues should remain face-builder/painter-owned preview details too, so nose/mouth plane readability improves without creating another controller-visible muzzle-state surface
  - lightweight forehead-pad cues should remain face-builder/painter-owned preview details too, so brow/eye-zone readability improves without creating another controller-visible forehead-state surface
  - lightweight temple-contour cues should remain face-builder/painter-owned preview details too, so eye-socket readability improves without creating another controller-visible temple-state surface
  - lightweight under-eye contour cues should remain face-builder/painter-owned preview details too, so eye-socket continuity improves without creating another controller-visible under-eye-state surface
  - lightweight nose-bridge cues should remain face-builder/painter-owned preview details too, so brow-to-muzzle continuity improves without creating another controller-visible nose-bridge-state surface
  - lightweight belly/flank contour cues should remain frame-builder/painter-owned preview details too, so torso volume improves without creating another controller-visible body-contour-state surface
  - lightweight sternum/torso-centerline cues should remain frame-builder/painter-owned preview details too, so chest-to-belly continuity improves without creating another controller-visible torso-centerline-state surface
  - lightweight upper-torso contour cues should remain frame-builder/painter-owned preview details too, so shoulder-to-chest continuity improves without creating another controller-visible upper-torso-state surface
  - lightweight back-contour cues should remain frame-builder/painter-owned preview details too, so torso front/back readability improves without creating another controller-visible rear-torso-state surface
  - lightweight hand/leg root-cuff cues should remain appendage-builder/painter-owned preview details too, so limb attachment continuity improves without creating another controller-visible limb-root-state surface
  - lightweight hand/leg silhouette-bridge cues should remain appendage-builder/painter-owned preview details too, so limb outer-contour continuity improves without creating another controller-visible limb-silhouette-state surface
  - action-aware proportion tuning should remain frame/appendage-builder-owned, so head scale, hand reach, and leg stance can bias by runtime state without creating another controller-visible proportion-state surface
  - action-aware body-stance tuning should remain frame-builder-owned, so torso width/height/center lift can bias by runtime state without creating another controller-visible body-stance-state surface
  - action-aware appendage-proportion tuning should remain appendage-builder-owned, so tail width/height and ear spread/lift can bias by runtime state without creating another controller-visible appendage-proportion-state surface
  - action-aware atmosphere-grounding tuning should remain frame-builder-owned, so glow/shadow/pedestal scale can bias by runtime state without creating another controller-visible atmosphere-grounding-state surface
  - action-aware atmosphere-grounding offset tuning should remain frame-builder-owned, so shadow/pedestal placement can bias by runtime state without creating another controller-visible atmosphere-grounding-offset-state surface
  - action-aware atmosphere-grounding weight tuning should remain palette/frame-owned, so shadow/pedestal alpha can bias by runtime state without creating another controller-visible atmosphere-grounding-weight-state surface
  - action-aware upper-atmosphere tuning should remain frame-builder-owned, so glow alpha/offset can bias by runtime state without creating another controller-visible upper-atmosphere-state surface
  - click/scroll atmosphere tuning should remain frame/palette-owned, so glow/shadow/pedestal responses can expand to those states without creating another controller-visible atmosphere-state matrix
  - atmosphere tint tuning should remain palette-owned, so shadow/pedestal hue can bias by runtime state without creating another controller-visible atmosphere-tint-state surface
  - the first real-renderer requirement seam is now active too:
    - `Win32MouseCompanionRealRendererAssetResources`
    - it adapts shared `model / action_library / appearance_profile` lanes into a renderer-facing resource contract
  - the second real-renderer requirement seam is now active too:
    - `Win32MouseCompanionRealRendererSceneRuntime`
    - it adapts shared runtime action / pose / facing state into a renderer-facing scene-runtime contract
  - the third seam is now active too:
    - `Win32MouseCompanionRealRendererSceneBuilder`
    - `Win32MouseCompanionRealRendererPainter`
    - `Win32MouseCompanionRealRendererBackend::Render(...)`
    - the current output is still a preview backend, but it now renders a stylized pet-like scene with ears/limbs/face/accessory markers instead of only abstract readiness geometry
    - action-specific visual overlays are now part of that preview contract too, and they intentionally stay renderer-owned rather than leaking back into controller logic:
      - `click` -> ring overlay
      - `hold` -> grip band
      - `scroll` -> orbit arc
      - `follow` -> trail overlay
      - `drag` -> motion slash
      - overlay stroke/alpha emphasis is now also renderer-owned, so preview intensity changes remain visible without adding a second controller-side visual-strength contract
    - face expression is now part of the same renderer-owned preview contract:
      - brows tilt/lift per action
      - mouth arc shape changes per action
      - blush intensity can vary with reactive/click state
    - whole-body posture is now part of that renderer-owned preview contract too:
      - body center can lift/drop per action
      - head can nod/lean independently
      - tail can lift/sag
      - shadow scale can compress/spread with the apparent weight shift
      - limb placement can change with follow/hold cadence
    - silhouette emphasis is now part of that renderer-owned preview contract too:
      - body/head/limb stroke weight can vary with action intensity
      - chest emphasis can tighten/soften with action state
    - atmosphere emphasis is now part of that renderer-owned preview contract too:
      - glow size can expand with action intensity
      - shadow/pedestal alpha can respond to follow/hold emphasis
      - accent visibility can strengthen with action intensity
    - action rhythm is now part of that renderer-owned preview contract too:
      - click can carry a rebound pulse
      - hold can carry a squeeze cadence
      - scroll can carry a bob/swirl cadence
      - drag can carry a pull pulse
      - follow can carry a gait rhythm
    - appendage coordination is now part of that renderer-owned preview contract too:
      - ears, tail, hands, and legs can reuse those action phases instead of only reading independent static offsets
    - idle life rhythm is also part of the preview contract:
      - lightweight breathing can modulate body/shadow scale
      - ears can keep a subtle cadence
      - tail can sway slightly
      - hands can float slightly
      - this rhythm should remain renderer-local and reuse existing runtime ticks rather than requiring a second idle animation subsystem
    - motion tuning now has a dedicated `Win32MouseCompanionRealRendererMotionProfile` seam:
      - it owns action-strength curves and idle rhythm shaping
      - `SceneBuilder` should consume that profile rather than growing a second formula bucket
    - overlay geometry now has a dedicated `Win32MouseCompanionRealRendererActionOverlayBuilder` seam:
      - it owns `click / hold / scroll / drag / follow` overlay placement and shape rules
      - `SceneBuilder` should delegate overlay assembly there so body/head/limb layout stays the stable core scene seam
    - face geometry now has a dedicated `Win32MouseCompanionRealRendererFaceBuilder` seam:
      - it owns brow/eye/mouth/blush placement derived from the motion profile
      - `SceneBuilder` should delegate expressive face assembly there so posture/layout and expression tuning evolve independently
    - accessory/badge assembly now has a dedicated `Win32MouseCompanionRealRendererAdornmentBuilder` seam:
      - it owns lane badges, pose badge, and accessory marker placement
      - `SceneBuilder` should delegate those adornment concerns there so the core scene seam stays centered on body/head/limb geometry
    - palette assignment now has a dedicated `Win32MouseCompanionRealRendererPaletteBuilder` seam:
      - it owns skin/theme/status-derived colors and material-like fill/stroke choices
      - `SceneBuilder` should delegate those presentation choices there so geometry and visual theming remain independently tunable
      - renderer-owned palette tokens now also travel through `Win32MouseCompanionRealRendererPaletteProfile`, so theme constants can evolve without reintroducing color literals into the builder itself
    - appendage geometry now has a dedicated `Win32MouseCompanionRealRendererAppendageBuilder` seam:
      - it owns tail/ear/hand/leg placement driven by pose samples and the motion profile
      - `SceneBuilder` should delegate that appendage assembly there so the core scene seam stays focused on body/head/frame layout
    - core frame geometry now has a dedicated `Win32MouseCompanionRealRendererFrameBuilder` seam:
      - it owns body/head/shadow/pedestal layout derived from runtime-facing momentum and the motion profile
      - `SceneBuilder` should delegate that frame assembly there so top-level orchestration remains a composition layer rather than another long-lived geometry bucket
    - shared layout metrics now have a dedicated `Win32MouseCompanionRealRendererLayoutMetrics` contract:
      - builder seams should exchange stable body/head sizing conventions through that struct rather than passing multiple bare float dimensions
      - this keeps future preview geometry evolution localized and makes builder signatures less brittle
    - shared visual ratios now have a dedicated `Win32MouseCompanionRealRendererStyleProfile` contract:
      - builder seams should consume common ratio/scale defaults there rather than duplicating ring/face/body/appendage/adornment sizing constants in each file
      - this keeps future Windows preview tuning centralized and avoids re-coupling geometry seams through copied magic numbers
      - current frame/appendage/face/adornment/overlay builders are expected to keep migrating new visual ratios into that style contract instead of reintroducing local hardcoded scale clusters
      - current face-expression anchors, accessory-star offset rules, and frame/palette tuning values now also belong to that style contract, so preview tuning can stay centralized without reopening builder-local geometry literals
  - current rollout rule:
    - default `real` availability is gated by `MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE`
    - when unset, diagnostics should report `unavailable_reason=rollout_disabled`
    - when set to `1/true/on/yes`, the preview backend becomes selectable for explicit testing
  - both `renderer_backend_catalog[*].unmet_requirements` and top-level `real_renderer_unmet_requirements` should stay stable enough for future automation/manual validation
  - current test-friendly preference source is `env:MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND`; when unset, diagnostics report `preferred_renderer_backend_source=default`
  - configured-preference diagnostics tell us whether config-backed preference resolution is currently active; they do not imply that the final selected backend matched the preferred backend
  - runtime-config preference updates now re-enter backend selection after host start; if a replacement backend cannot be created, the current backend stays alive so the host does not drop rendering during preference experiments
  - backend preference normalization is now separated from the factory path, so future config-backed preference sources can reuse the same canonical `default -> auto` and lowercase/trim rules
- Backend lifecycle is now part of the contract too:
  - renderer backend construction alone is not treated as readiness
  - backends are expected to implement `Start()`, `Shutdown()`, `IsReady()`, and `LastErrorReason()`
  - future real backends should surface initialization failures through `LastErrorReason()` so factory fallback remains local to backend selection
- Backend preference source routing is now explicit too:
  - preference resolution no longer assumes only one hardcoded env source
  - current built-ins are ordered as `configured_request -> env -> default`
  - future config-backed preference sources should plug into the registry layer instead of expanding factory-side conditionals
  - explicit caller-provided preference requests now reuse that same registry path, so source precedence stays uniform across test/debug/runtime entrypoints
  - Windows visual host now forwards an internal runtime-config preference request before backend creation; this is intentionally an internal seam first, not a user-facing schema flag yet
  - hidden config/json fields now persist backend preference source/name so future settings-backed preference can be enabled without changing runtime/request plumbing again
  - hidden settings/apply/diagnostics lanes now round-trip those fields too, so future renderer preference rollout no longer depends on adding a new transport path
- Current limitation remains explicit:
  - the placeholder renderer is still the only backend implementation
  - built-in backend selection now routes through an explicit backend registry + registration step, so future Windows backends no longer need another factory/window rewrite
  - presenter/runtime/host behavior is unchanged
  - this step is an architectural handoff seam, not a visual-capability upgrade yet

## Regression Rule
Any future Windows real-renderer landing must preserve:
- shared `IPetVisualHost` usage from `AppController`
- current placement/clamp semantics
- current diagnostics field meanings
- current asset-lane separation (`model / action_library / appearance_profile`)
- current runtime semantic action labels
