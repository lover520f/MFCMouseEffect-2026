# Dawn Hold Trail Priority Submit Simplify (Stage 135)

## Background
- On the Dawn compositor path, `DawnCommandConsumer` still had a double-submit pattern on geometry frames:
  - first `TrySubmitNoopQueueWork`
  - then per-pass empty command buffer submit via packet submit helpers
- Under `hold_neon3d` mixed frames (`trail + hold ripple`), non-trail packet submits were still too frequent, while trail updates were throttled too aggressively, causing visible cursor-follow lag.

## Root-Cause-Oriented Changes
- Removed redundant noop submit from the geometry submit path in `DawnCommandConsumer`:
  - geometry frames now submit directly through the existing packet submit path.
  - warmup/no-geometry probes still keep their original behavior.
- Added hold mixed-frame non-trail submit gating:
  - when `hold + trail + non-trail` are all present, non-trail packet submit is rate-limited to `8ms`.
  - trail packet remains priority each frame.
- Reduced hold-time trail skip interval:
  - `kTrailSubmitIntervalWhenHoldMs`: `20ms -> 8ms`
  - objective: reduce visible trail catch-up lag during rapid hold movement.

## Expected Impact
- Lower submission overhead on active geometry frames (fewer redundant queue submits).
- Better hold-path responsiveness: trail stays smoother and closer to cursor movement.
- Keeps CPU fallback architecture intact; only Dawn command consumer scheduling/submit behavior changed.

## Validation
- Build:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect.slnx /t:Build /p:Configuration=Release /p:Platform=x64 /m /nologo`
  - Result: success (`0 error`).
- Link lock handling:
  - hit `LNK1104` once due to running `MFCMouseEffect.exe`.
  - resolved by agent-side process termination and immediate rebuild (no user action required).

