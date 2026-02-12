# Dawn diagnostic timeline retention expansion (Stage 89)

## Problem
During manual validation, hold interaction samples were frequently missing from the final diagnostic timeline snapshot.

The previous timeline size (`180`) could be overwritten quickly by subsequent hover/idle frames, making post-action analysis unreliable.

## Changes
- File: `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
1. Expanded timeline retention:
   - `kDawnConsumerTimelineMax` from `180` to `1800`

## Why this design
- Keeps existing schema and diagnostics pipeline intact.
- Greatly improves observability for short-lived hold bursts after user actions.
- Zero behavior impact on rendering path; this is diagnostics-only retention.

## Validation
1. Build with VS2026 Professional MSBuild succeeded.
2. Web state `dawn_command_consumer_timeline_max` now reports larger retention window.
