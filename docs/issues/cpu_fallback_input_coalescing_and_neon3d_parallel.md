# CPU Fallback Optimization: Input Coalescing + Neon3D Parallel Geometry

## Why
- Under heavy effects (for example Trail `tubes` and Hold `neon HUD 3D`), the dispatch queue could accumulate many `WM_MFX_MOVE` messages.
- That queue pressure increased apparent cursor lag and frame drops in CPU fallback mode.

## Change 1: Mouse Move Coalescing
- File: `MFCMouseEffect/MouseFx/Core/GlobalMouseHook.h`
- File: `MFCMouseEffect/MouseFx/Core/GlobalMouseHook.cpp`
- File: `MFCMouseEffect/MouseFx/Core/AppController.cpp`

### Before
- Every low-level mouse move posted one `WM_MFX_MOVE` message.
- Dispatch thread processed each message serially.

### After
- Hook stores latest move point atomically.
- At most one pending `WM_MFX_MOVE` is posted.
- Dispatch consumes the latest point via `ConsumeLatestMove`.
- Effect behavior stays responsive while dropping redundant intermediate move events.

## Change 2: Neon3D Branch Geometry Parallel Build
- File: `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h`

### Before
- Branch geometry (`DrawBranchTendrils`) was built sequentially per branch.

### After
- Branch geometry generation is split per branch and built with `Concurrency::parallel_for` when branch count is high enough.
- GDI+ drawing remains on the render thread; only pure math/geometry generation is parallelized.

## Design Notes
- This keeps CPU fallback quality intact (no quality downgrade).
- This is compatible with a future GPU backend (`wgpu`): input coalescing and data-prep split remain useful regardless of renderer backend.
