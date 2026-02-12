# Stage 95 - Web State Slim Payload + Local Full Diagnostic Snapshot

## Background
- The settings web page was still heavy and could stutter while polling `/api/state`.
- Root cause: `/api/state` serialized a large `dawn_command_consumer_timeline` (up to 1800 points) on every poll.
- Requirement: keep essential UI data, move heavy debug data to local untracked files for developer-side analysis.

## What Changed
- Added timeline tail API:
  - `GetDawnCommandConsumeTimelineTail(size_t maxPoints)` in `MouseFx/Gpu/DawnCommandConsumer.h`.
- Added snapshot gating/writer split:
  - `LocalDiagStateWriter::ShouldWriteSnapshot(...)`
  - `LocalDiagStateWriter::WriteSnapshotNow(...)`
  - keep `TryWriteSnapshot(...)` as compatibility wrapper.
- `WebSettingsServer::BuildStateJson()` now uses dual output:
  - Web response:
    - keeps full `dawn_command_consumer` summary.
    - `dawn_command_consumer_timeline` only keeps tail (64 points).
    - adds `dawn_command_consumer_timeline_tail_max=64`.
    - marks `dawn_command_consumer_timeline_truncated=true`.
  - Local diagnostic dump (throttled):
    - `web_state_auto.json` stores full timeline + runtime symbol detail.
    - `web_state_slim_auto.json` stores the slim payload returned to UI.

## Why This Is Root-Cause Oriented
- We did not patch rendering symptoms in UI.
- We reduced state construction complexity at the source:
  - less JSON generation and transport cost per poll.
  - heavy diagnostics kept available locally for development and replay.

## Validation
- Build target: `Release|x64`.
- Result: compile/link success with 0 errors.
- Runtime expectation:
  - settings UI receives short timeline payload.
  - local `.local/diag/web_state_auto.json` keeps full diagnostics.

