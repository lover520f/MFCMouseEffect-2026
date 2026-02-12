# Dawn GPU Startup First-Frame Pre-Render (Stage 123)

## Summary
Added first-frame pre-render in overlay startup flow so window surfaces are populated before being shown.

## Why
A visible black flash can happen when overlay windows are shown before first render pass completes, especially during startup or surface rebuild transitions.

## What changed
1. `OverlayHostWindow::StartFrameLoop` now:
- updates alive layers once
- collects GPU command stream once
- renders one frame once
- then shows windows (`SW_SHOWNA`)

2. Existing timer loop remains unchanged for steady-state rendering.

## Safety
- No behavior change to fallback policy.
- No architecture bypass; only startup ordering changed.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64).

## Next
Re-check startup black flash frequency and correlate with latest diag timestamp after this commit.
