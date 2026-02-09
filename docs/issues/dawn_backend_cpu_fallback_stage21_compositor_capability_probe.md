# Dawn Backend + CPU Fallback (Stage 21 Compositor Capability Probe)

## Goal
- Add explicit capability probes for upcoming Dawn compositor path.
- Surface compositor readiness in runtime/bridge/API/UI so next implementation step is measurable.

## Changes

### 1) Dawn Runtime Probe Expansion
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added probe flags:
  - `hasCreateSurface`
  - `hasGetQueue`
  - `hasSurfacePresent`
- Probe now dynamically checks corresponding Dawn/WebGPU symbols.

### 2) Bridge Capability Enrichment
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.cpp`
- Added bridge fields:
  - `compositorApisReady`
  - `compositorDetail`
- Current rule:
  - compositor APIs are marked ready when required symbols exist and device creation succeeded.

### 3) API / Schema / UI Visibility
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `MFCMouseEffect/WebUI/app.js`
- API payload adds:
  - compositor-related probe bits in `dawn_probe`
  - bridge compositor fields in `dawn_overlay_bridge`
- Schema adds:
  - `gpu_status_schema.compositor_codes`
- UI banner adds:
  - `Compositor API: Ready/Not Ready` (en/zh wrapper)

## Result
- Compositor path prerequisites are now observable without changing current rendering path.
- Stage22 can focus on implementing actual Dawn surface/present flow with clear readiness gating.
