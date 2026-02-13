# GPU Route Switch Stage 31: Multi-Frame Trial Clear After Hold End

Date: 2026-02-13

## Goal
Eliminate residual trail/stale image left on visible-trial layer after hold release.

## Root Cause
Single-frame clear on hold-end was sometimes insufficient for composition timing, leaving stale content visible.

## Changes
- Replaced one-shot clear flag with a multi-frame clear counter:
  - `holdNeon3dGpuTrialClearFramesPending_`
- On hold transition `active -> inactive`, schedule 3 clear uploads.
- Trial upload condition now allows:
  - hold active, or
  - pending clear frames > 0.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - no lingering trail residue after hold release in visible-trial fallback path.

## Risk
- Low. Adds two extra clear uploads after hold release; no change to authoritative layered present path.
