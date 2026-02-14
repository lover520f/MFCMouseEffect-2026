# Stage 39 - FluxField GPU Visibility Fix with Matrix-Anchor Fallback

## Problem
- FluxField GPU route could become invisible in some sessions.
- Root symptom: cursor-state mapping could land off-surface, while renderer treated this as a successful no-op draw.

## Root cause
- In GPU D2D backend, when `hold_state` cursor converted outside current surface bounds, code exited render as `ok=true` without drawing.
- Across monitor/surface calls, this could produce no visible output.

## Fix
1. Keep cursor-state anchor when mapping is valid and in-surface.
2. If cursor mapping is off-surface, do **not** short-circuit as success.
3. Fall back to matrix-derived anchor (world transform translation) and continue rendering.
4. If the final clip region becomes empty (`rcRight <= rcLeft` or `rcBottom <= rcTop`), treat this frame as a render failure (not success) so upper layer can switch to CPU fallback instead of silently outputting nothing.

## Files changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h`

## Validation
1. Select `FluxField HUD GPU (Auto Fallback CPU)`.
2. Long-press and move across displays / edges.
3. Effect should remain visible instead of disappearing silently.
