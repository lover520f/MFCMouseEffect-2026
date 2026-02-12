# Dawn Trail Cap Observability (Stage 158)

## Background
- Trail cap policy has multiple branches (hold / mixed non-hold / pure non-hold).
- Diagnostics previously showed geometry results but not the selected cap branch directly.

## Change
- Add explicit cap decision fields into `dawn_command_consumer` output:
  - `trail_vertex_cap_per_command`
  - `trail_cap_mode` (`hold` / `mixed_non_hold` / `trail_non_hold` / `none`)

## Why
- Make post-test diagnosis deterministic and faster.
- Avoid inferring cap branch indirectly from geometry numbers.

## Validation
- Build `Release|x64`.
- Verify `web_state_auto.json` contains new keys under `dawn_command_consumer`.
