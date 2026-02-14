# Dawn Native Effects Route

Date: 2026-02-13

## Decision
- Stop using Dawn as a direct replacement path for existing GDI layered final-present semantics.
- Keep the current D3D11+DComp branch as an archived/stabilization line.
- Start a new branch from clean `main` for Dawn-native effects:
  - `dawn-native-effects-v1`

## Why Previous Dawn Route Failed
- Existing effect stack and final-present behavior are tightly coupled to GDI layered window semantics.
- Dawn/WebGPU render flow and layered `UpdateLayeredWindow` final composition are not the same presentation model.
- Forcing takeover in this mixed model caused instability risks (black screen, multimonitor artifacts, fallback thrash).

## New Route Principles
- Build new effects specifically for Dawn render model, not 1:1 migration of legacy GDI effects.
- Keep legacy GDI effects intact for stability baseline.
- Isolate Dawn pipeline by module boundary:
  - input/event bridge
  - effect state + parameter model
  - Dawn renderer backend

## First Milestone
- Implement one Dawn-native effect end-to-end:
  - `hold_neon3d_gpu_v2` (new effect id, independent from legacy `hold_neon3d`)
- Acceptance for milestone:
  - no black-screen regression
  - effect frame continuity under long hold + fast move
  - clear local diagnostics for route activation and fallback

## Branch Strategy
- `main`: stable baseline, no in-progress Dawn takeover experiments.
- `gpu-restart-d3d11`: archived compatibility/stabilization line.
- `dawn-native-effects-v1`: active development for Dawn-native new effects.
