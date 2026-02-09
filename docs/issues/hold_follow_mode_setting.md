# Hold Follow Mode Setting (Web Settings)

## Goal
- Expose a user-facing setting for hold-effect cursor tracking behavior.
- Different users can choose responsiveness vs stability vs CPU usage.

## UI Changes
- Added a new option in Web settings `General` card:
  - `Hold Tracking` (`hold_follow_mode`)
- Added a warning/info badge `!` beside the label.
- Hover tooltip explains tradeoffs:
  - precise: fastest response
  - smooth: balanced
  - efficient: lower CPU pressure

## 2026-02-09 UX Copy Refinement

- Improved tooltip copy from generic tradeoff text to scenario-based guidance:
  - when to choose low latency (`precise`)
  - which one is recommended default (`smooth`)
  - when to prioritize CPU budget (`efficient`)
- Updated schema option labels to make choice intent obvious in the dropdown:
  - `precise` -> `Precise (Low Latency)` / `精准跟随（低延迟）`
  - `smooth` -> `Smooth (Recommended)` / `平滑跟随（推荐）`
  - `efficient` -> `Performance First (Lower CPU)` / `性能优先（省CPU）`

## Runtime Modes
- `precise`
  - Always updates hold effect position with latest point.
  - Sends hold progress command every update.
- `smooth`
  - Uses lightweight interpolation for position.
  - Limits progress command frequency to reduce command pressure.
- `efficient`
  - Throttles hold position/progress update frequency.
  - Prioritizes CPU efficiency.

## Persistence + API
- New config field: `hold_follow_mode` in `config.json`.
- Included in:
  - `/api/schema` as `hold_follow_modes`
  - `/api/state` as `hold_follow_mode`
  - `/api/state` apply payload as `hold_follow_mode`
- AppController applies mode immediately and recreates active hold effect for instant take-effect.

## Backward Compatibility
- Missing/invalid values are normalized to `smooth`.
- Existing config files continue to work without migration steps.
