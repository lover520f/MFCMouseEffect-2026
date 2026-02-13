# GPU Route Switch Stage 11: Probe Composition Swapchain Trial

Date: 2026-02-13

## Goal
Validate end-to-end DirectComposition content path by attaching a real composition swapchain to the hidden probe visual and presenting once, without changing visible layered rendering.

## Changes
- Added hidden probe content chain creation in `D3D11DCompPresenter`:
  - `IDXGIFactory2::CreateSwapChainForComposition`
  - `IDCompositionVisual::SetContent(swapchain)`
  - `IDCompositionDevice::Commit()`
  - one probe `Present(0, 0)`
- Added status field:
  - `compositionSwapChainReady`
- Exposed `/api/state -> gpu_present_host.composition_swapchain_ready`.
- `TryActivateTakeoverPath()` now distinguishes:
  - swapchain creation failure -> auto-off marker with failure reason
  - swapchain success -> still fallback layered by policy in this stage

## Why
This stage removes a core uncertainty: whether DComp host can carry real swapchain content in runtime. It keeps user-visible behavior safe while proving takeover prerequisites.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - with takeover enabled path, status eventually reflects `composition_swapchain_ready=true` before fallback.

## Risk
- Low-medium. New COM/DXGI objects are created, but only on hidden probe path; visible rendering path remains layered.
