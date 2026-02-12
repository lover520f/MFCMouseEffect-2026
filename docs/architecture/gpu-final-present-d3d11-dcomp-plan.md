# GPU Final Present Plan (D3D11 + DirectComposition)

## Goal
- Achieve stable full GPU final present for transparent overlay behavior on Windows.
- Preserve deterministic CPU fallback for safety.

## Non-Goals (Stage 1)
- No visual takeover yet.
- No shader migration yet.

## Architecture
- `Renderer`: produces per-frame visual content (existing effect logic can be migrated gradually).
- `Presenter`: owns Windows final-present path:
  - D3D11 device/context
  - DXGI resources
  - DirectComposition device/target/visual chain
- `Policy`: enables takeover only when presenter host is ready and stable.

## Migration Steps
1. Device bring-up scaffold (this stage).
2. Hidden host-chain activation + diagnostics.
3. Single-monitor takeover with hard rollback.
4. Multi-monitor takeover with per-surface safety gate.
5. Enable by policy; keep CPU fallback as default recovery path.

## Guardrails
- Any present failure increments rollback counters and returns to CPU path.
- No hidden dynamic heuristics as primary gate; readiness should be explicit and deterministic.
