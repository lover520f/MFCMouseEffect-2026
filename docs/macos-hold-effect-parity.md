# macOS Hold Effect Parity

## Overview
All 8 hold effect styles now have per-type rendering on macOS, matching their Windows visual contracts.

## Architecture
```
Core (Compute):
  HoldEffectStartCommand → strokeArgb, sizePx, timing, colors, normalizedType
  HoldEffectUpdateCommand → holdMs, overlayPoint, progress

Platform macOS (Render):
  MacosHoldPulseOverlayRendererCore.Start.cpp  → C bridge to Swift
  MacosHoldPulseOverlayBridge.swift            → per-type CALayer builders + dispatch
  MacosHoldPulseOverlayRendererCore.Update.cpp → passes updateCmd to Swift
```

## Per-type Visual Mapping

| Style | Windows Renderer | macOS Builder | Key Layers |
|---|---|---|---|
| charge | ChargeRenderer | `mfxBuildChargeLayers` | stroke ring + glow arc + progress arc + dot head |
| lightning | LightningRenderer | `mfxBuildLightningLayers` | radial gradient orb + 24 bolt lines + core pulse |
| hex | HexRenderer | `mfxBuildHexLayers` | 3 hexagons (0.4/0.7/1.0×r) + vertex dots + rotation |
| tech_ring | TechRingRenderer | `mfxBuildTechRingLayers` | 3D perspective + 3 tilted rings + 20 orbit dots |
| hologram | HologramHudRenderer | `mfxBuildHologramLayers` | segmented rings (3+4) + 15 rising particles |
| flux_field | FluxFieldHudCpuRenderer | `mfxBuildFluxFieldLayers` | 6 rings + 4 arc bands + 20 dots + center glow |
| neon3d | Neon3DRenderer | `mfxBuildNeonLayers` | glass ring + scanner + progress arc + crystal seed |
| quantum_halo | (GPU D3D11) | uses fluxField | GPU-only on Windows; CA fallback on macOS |

## Layer Naming Convention
- `mfx_hold_ring` – primary container / ring handle
- `mfx_hold_accent` – secondary container / accent handle
- `mfx_hold_progress` – strokeEnd-based progress arc
- `mfx_hold_head` – orbiting dot (charge/neon only)

## Key Decision
`ring` and `accent` handles cast as `CALayer` (not `CAShapeLayer`) in update function for type safety across styles where ring may be `CAGradientLayer`.
