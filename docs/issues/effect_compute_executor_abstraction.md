# Effect Compute Executor Abstraction

## Problem
- Heavy effects were directly using concrete threading APIs (for example `Concurrency::parallel_for`) inside effect/render code.
- This couples effect logic with threading details and makes future migration harder.

## Goal
- Effects only describe "what to compute" (input -> output build function).
- Threading policy is handled by one centralized executor utility.

## Added
- `MFCMouseEffect/MouseFx/Compute/EffectComputeExecutor.h`
  - `mousefx::compute::BuildArray<T>(count, parallelThreshold, buildFn)`
  - Centralizes serial/parallel decision and execution.

## First Migration
- `MFCMouseEffect/MouseFx/Renderers/Hold/Neon3D/Neon3DFx.h`
  - Branch geometry generation now calls `BuildArray`.
  - Renderer logic no longer directly references `parallel_for`.

## Benefits
- Single place to evolve CPU scheduling strategy.
- Easier to add telemetry/budgeting/throttling later.
- Smooth path to future backend split (CPU fallback vs GPU pipeline).
