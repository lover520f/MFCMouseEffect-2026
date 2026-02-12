# GPU Route Switch: Dawn to D3D11+DComp (Stage 1)

## Background
- The previous Dawn route proved that command preprocessing and diagnostics can work.
- The blocking issue remained final-present takeover stability under transparent multi-monitor overlay behavior.

## What We Learned
- `GPU command ready` does not mean `GPU final present ready`.
- Layered-window semantics and final present takeover are the hardest part of this app architecture.
- Stability must be the first gate; fallback must stay deterministic.

## Decision
- Freeze Dawn takeover as an archive path (`dawn-archive` branch).
- Restart GPU landing from clean `main` with Windows-native final-present host:
  - D3D11 device + DXGI
  - DirectComposition visual chain

## Stage-1 Implementation
- Add a compile-ready `D3D11DCompPresenter` scaffold:
  - brings up D3D11 device/context
  - queries DXGI device
  - creates DirectComposition device
- No runtime takeover yet; this stage is bring-up only.

## Why This Stage Exists
- Split bring-up risk from behavior risk.
- Keep the current app stable while preparing a deterministic final-present path.
