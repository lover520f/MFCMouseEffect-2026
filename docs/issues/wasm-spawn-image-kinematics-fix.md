# WASM spawn_image Kinematics Fix

## Symptom

WASM image effects looked static at click position:
- image appears,
- but no visible trajectory,
- users perceive this as "no image effect".

## Root Cause

Two issues were stacked:

1. Host execution path did not apply `spawn_image` motion semantics.
- `SpawnImageCommandV1` contains `vx/vy/ax/ay/delayMs`.
- `WasmClickCommandExecutor` previously passed only direction/intensity.
- `RippleOverlayLayer` rendered instance center at fixed click point.

2. Several template image samples had near-cancelled trajectories.
- low negative `vy` with high positive `ay` produced near-zero net displacement over lifetime.

## Changes

## 1. Motion and delay moved into host runtime path

Updated `RenderParams` and overlay runtime to support optional kinematics:
- `useKinematics`
- `velocityX/velocityY`
- `accelerationX/accelerationY`
- `startDelayMs`

Files:
- `MFCMouseEffect/MouseFx/Interfaces/IRippleRenderer.h`
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h`
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`

Behavior:
- when `useKinematics` is true, render center is updated by
  `dx = vx*t + 0.5*ax*t^2`, `dy = vy*t + 0.5*ay*t^2`;
- `delayMs` now postpones visibility without dropping the instance;
- existing non-WASM effects remain unchanged (default kinematics off).

## 2. Template sample trajectories re-tuned

Adjusted image-heavy samples to have clearly visible motion:
- `image-pulse.ts`
- `image-burst.ts`
- `image-lift.ts`
- `mixed-text-image.ts`
- `mixed-emoji-celebrate.ts`
- `button-adaptive.ts`

## Validation

- `pnpm run build:samples` (template): all sample bundles succeeded.
- `MSBuild x64 Debug` (solution): build passed with 0 errors.

## Impact

`spawn_image` now behaves as a real effect command:
- visible movement,
- delay semantics honored,
- better sample defaults for user perception.
