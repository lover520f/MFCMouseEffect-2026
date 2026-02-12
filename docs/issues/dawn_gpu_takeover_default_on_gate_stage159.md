# Dawn GPU Takeover Default-On Gate (Stage 159)

## Background
- Final GPU present takeover was gated by a strict local `.on` file requirement.
- In practice, this kept many runs permanently in layered CPU final-present even when runtime and host chain were ready.

## Change
- Update takeover gate policy to default-on when takeover integration is built and runtime gates are satisfied.
- Keep `.off` file as a hard local kill switch.
- Keep `.on` file support for explicit intent; diagnostics now distinguish:
  - `takeover_ready_explicit_on`
  - `takeover_ready_default_on`

## Why
- Remove hidden local config dependency that blocked full GPU landing.
- Preserve rollback and off-switch safety while enabling global-optimal default path.

## Validation
- Build `Release|x64`.
- Confirm diagnostics no longer require `.on` to reach takeover-ready status (unless `.off` exists).
