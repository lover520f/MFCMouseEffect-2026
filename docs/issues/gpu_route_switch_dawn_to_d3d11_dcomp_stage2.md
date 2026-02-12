# GPU Route Switch: Dawn to D3D11+DComp (Stage 2)

## Background
- Stage 1 introduced a compile-ready D3D11 + DirectComposition presenter scaffold.
- Next step is wiring it into host lifecycle without changing current stable present behavior.

## Change
- `OverlayHostWindow` now owns a `D3D11DCompPresenter`.
- Presenter device bring-up runs during host create.
- Presenter shutdown runs during host teardown.
- Current final-present path remains layered CPU (`UpdateLayeredWindow`) by design.

## Why
- Build the future takeover host chain incrementally with zero behavior regression.
- Keep rollout deterministic: lifecycle first, present path switch later.

## Validation
- Build `Release|x64`.
- Run app and verify behavior matches existing layered baseline (no takeover yet).
