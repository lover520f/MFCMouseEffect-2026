# Dawn Native Stage 1: `hold_neon3d_gpu_v2` Entry

Date: 2026-02-13

## Goal
Create an independent hold-effect id for the new Dawn-native route, without changing legacy `hold_neon3d` behavior.

## Changes
- Added new hold command id:
  - `kCmdHoldNeon3DGpuV2 = 7008`
- Added new hold metadata option:
  - value: `hold_neon3d_gpu_v2`
  - label: `Neon HUD GPU v2`
- Added dedicated renderer registration:
  - `hold_neon3d_gpu_v2`
  - current implementation delegates to `HoldNeon3DRenderer` as a stage-1 placeholder.
- Hooked tray command mappings:
  - label fallback
  - current-type match
  - IPC set_effect mapping

## Why
- We need a clean route id for iterative Dawn-native evolution.
- Keeping a separate effect id avoids coupling new-route experiments with legacy hold effect semantics.

## Validation
- Build target: `Release|x64` passes.
- Tray/Settings/Web schema can select `hold_neon3d_gpu_v2`.
- Selecting this id successfully activates a hold renderer path.

## Next
- Replace placeholder delegation with Dawn-native render backend implementation under the same effect id.
