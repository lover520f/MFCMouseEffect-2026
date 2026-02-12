# Dawn Layered Path Latency Tuning (Stage 110)

## Summary
Improved responsiveness while still on layered CPU final-present path by tightening host frame pump cadence when Dawn backend is active.

## What changed
1. `OverlayHostWindow` timer policy:
- keep hold mode at 4ms.
- when Dawn backend is active (even if final present is CPU fallback), use 4ms frame interval instead of 8ms.
2. Presenter detail clarity:
- when Dawn compositor mode is selected but layered host enforces CPU final present, report
  `gpu_present_disabled_layered_host_requires_cpu_final_present`.
3. GPU banner action clarity:
- for `gpu_backend_active_cpu_present`, action text now explicitly states that GPU command path is active but final present remains layered CPU fallback.

## Files
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Validation
- `Release|x64` build passed.

## Expected effect
- Lower interaction-to-visual delay for trail/ripple during Dawn backend active sessions.
- Diagnostics now explicitly describe current architectural limit instead of suggesting probe actions.
