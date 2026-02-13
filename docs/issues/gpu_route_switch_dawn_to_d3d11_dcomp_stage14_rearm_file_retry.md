# GPU Route Switch Stage 14: Rearm File for Controlled Retry

Date: 2026-02-13

## Goal
Allow controlled retry of takeover trials without manual code changes by consuming a local rearm file.

## Changes
- Added rearm trigger file:
  - `<exe_dir>/.local/diag/gpu_final_present_takeover.rearm`
- On startup, presenter consumes this file and clears:
  - `gpu_final_present_takeover.off.disabled_by_codex`
- Added status field:
  - `rearmProcessed`
- Exposed `/api/state -> gpu_present_host.rearm_processed`.

## Why
During gated rollout, automatic fuse-off protects stability but can slow iterative verification. Rearm file provides an explicit, auditable way to retry safely.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - create `.rearm` file and restart app -> `rearm_processed=true` once, and auto-off marker is cleared.

## Risk
- Low. Control-plane only; trial path is still hard-gated and layered fallback remains authoritative.
