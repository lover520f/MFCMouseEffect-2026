# Stage 31 - Neon GPU-v2 Formal HUD3D Shader Upgrade

## Goal
- Replace the temporary test-like ring visual with a formal Neon HUD3D look while keeping the full GPU route.
- Preserve compatibility and performance from stage29/30 (`D3D11 + DXGI + DComp`, no D2D surface bridge).

## What changed
- Updated pixel shader in:
  - `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2ShaderPipeline.h`

## Visual layers added in shader
- Glass shell ring with inner/outer rim highlights.
- 12 o'clock anchored scanner and outlet line.
- Arc-anchored hatch texture for inner wall motion.
- Energy spokes and woven band (head-intensified).
- Progress arc + capsule head highlight.
- Central crystal seed (diamond + core glow).

## Color behavior
- Switched to a stable Neon HUD palette (cyan/purple/mint) with light user stroke tinting.
- Avoids the previous “red test circle” appearance even when theme stroke is warm.

## Transparency / compositing
- Keeps premultiplied-alpha output for composition swapchain.
- Uses radial envelope and low-alpha discard to avoid dark rectangular footprint.

## Performance notes
- Still a single full-screen triangle draw call per frame.
- No extra CPU path or GDI fallback in this presenter.
- Shader math is heavier than stage30 test style, but remains bounded and GPU-friendly.

## Validation
1. Build `Release|x64`.
2. Select `hold_neon3d_gpu_v2` and long-press.
3. Expected:
   - formal Neon HUD3D look (not simple ring test style)
   - stable rendering and low stutter
   - GPU route still active
