# Dawn Hold Trail Visibility Recovery (Stage 136)

## Symptom
- During hold interactions (especially `hold_neon3d`), trail appeared to disappear while hold effect was active.

## Diagnosis
- From local diagnostics (`web_state_full_auto.json`):
  - hold frames were mostly `ripple_hold_commands > 0` with `trail_commands = 0`.
- Root cause was not Dawn submit rejection.
- Root cause was trail-layer policy:
  - `TrailOverlayLayer` returned early in `Update/Render/AppendGpuCommands` when `latencyPriorityMode` was enabled.
  - `latency_priority` enabling also forced an immediate `clear`, making continuity worse.

## Fix
- Keep trail path active in latency-priority mode:
  - removed early returns in `TrailOverlayLayer::Update`
  - removed early returns in `TrailOverlayLayer::Render`
  - removed early returns in `TrailOverlayLayer::IntersectsScreenRect`
  - removed early returns in `TrailOverlayLayer::AppendGpuCommands`
- Keep latency-priority as "lighter fallback sampling" rather than "pause":
  - fallback cursor sampling interval now uses `32ms` in latency-priority mode (default remains `24ms`).
- Removed forced trail clearing when enabling latency-priority:
  - `TrailEffect::OnCommand("latency_priority", "on")` no longer calls `Clear()`.
  - `AppController::SetTrailLatencyPriorityMode(true)` no longer sends extra `clear` command.

## Result
- Hold effect and trail can coexist again under Dawn layered fallback path.
- Hold-latency optimization remains, but no longer sacrifices trail visibility.

## Validation
- Build:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect.slnx /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo`
  - Result: success (`0 error`).

