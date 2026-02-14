# Dawn Native Stage 5: Scope Route Snapshot to Hold Category

Date: 2026-02-13

## Goal
Prevent GPU route diagnostics from being overwritten by non-hold effect changes.

## Changes
- Updated `WriteGpuRouteStatusSnapshot(...)` to include category and only persist snapshot for `EffectCategory::Hold`.
- Added `category` field in `gpu_route_status_auto.json` output (`"hold"`).

## Why
- Previous implementation wrote snapshot for all categories.
- Recent operations like hover/click updates could overwrite the latest hold-route result, making diagnostics misleading.

## Validation
- Build `Release|x64` succeeds.
- `gpu_route_status_auto.json` now reflects hold route transitions only.
