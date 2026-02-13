# Stage 33 - Quantum Halo GPU-v2 Rename, Compatibility Alias, and High-Fidelity Shader Upgrade

## Why
- The previous effect id/name still carried `Neon HUD` semantics, but the actual visual and route implementation had diverged.
- Users needed clear naming in tray/web settings and stable compatibility with existing configs.
- The GPU visual needed a more advanced look that would be expensive to reproduce on CPU.

## What changed
- Renamed user-facing hold effect identity to `Quantum Halo GPU v2`.
- Kept backward compatibility for old config id `hold_neon3d_gpu_v2`.
- Upgraded the D3D11 pixel shader to a heavier multi-layer visual:
  - dual rotating FBM fields
  - arc head + trailing capsule energy
  - ring caustics and spoke lattices
  - reactor-core turbulence and sparkle layer
  - strict premultiplied-alpha output to avoid dark rectangular footprint

## Compatibility strategy
- Old id `hold_neon3d_gpu_v2` is normalized to `hold_quantum_halo_gpu_v2` in runtime routing.
- Old id is also accepted in:
  - hold direct runtime type checks
  - tray checked-state matching
  - renderer registry alias
- Result: old config can continue to run, and persisted runtime type converges to the new id.

## Files changed
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- `MFCMouseEffect/MouseFx/Effects/HoldEffect.h`
- `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/QuantumHaloGpuV2ShaderPipeline.h`
- `MFCMouseEffect/Settings/SettingsOptions.h`
- `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`

## Validation checklist
1. Build `Release|x64`.
2. Select hold effect `Quantum Halo GPU v2` from tray/settings web UI.
3. Long-press mouse:
   - effect is visible and centered at cursor
   - no white sector artifact
   - no dark rectangular background block
4. Old config id regression:
   - set hold type to `hold_neon3d_gpu_v2`
   - verify effect still renders and route remains GPU direct runtime

## Risk notes
- Shader complexity increased intentionally; low-end GPU devices may show lower frame rate under stress.
- Route fallback logic is unchanged in principle; this stage focuses on naming coherence + visual quality + compatibility aliasing.
