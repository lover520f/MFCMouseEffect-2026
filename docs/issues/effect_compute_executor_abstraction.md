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

## Second Migration
- `MFCMouseEffect/MouseFx/Renderers/Trail/TubesRenderer.h`
  - Tube chain physics update now calls `BuildArray`.
  - Renderer only describes per-chain transition (`BuildUpdatedChain`), not threading APIs.
  - Chain states are computed first, then swapped back in one step.

## Third Migration
- `MFCMouseEffect/MouseFx/Renderers/Hover/TubesHoverRenderer.h`
  - Node frame precompute (position/radius/base color) now calls `BuildArray`.
  - Rendering keeps main-thread GDI+ drawing; compute stage is isolated behind executor.

## Policy Abstraction (No Per-Effect Magic Numbers)
- `MFCMouseEffect/MouseFx/Compute/EffectComputeExecutor.h`
  - Added `ParallelProfile`:
    - `AggressiveSmallBatch`
    - `Balanced`
    - `Throughput`
  - Added `ResolveParallelThreshold(...)` and `ShouldRunParallel(...)`.
  - Added overload:
    - `BuildArray<T>(count, ParallelProfile, buildFn)`
- Effects no longer pass raw threshold literals like `2/4/8`.
  - `TubesRenderer` -> `ParallelProfile::AggressiveSmallBatch`
  - `Neon3DFx` -> `ParallelProfile::Balanced`
  - `TubesHoverRenderer` -> `ParallelProfile::Throughput`

### Why this matters
- Effect code now only describes compute intent (small/medium/throughput), not thread scheduling numbers.
- Future tuning (CPU fallback behavior per machine/core count) can be done centrally in one file.

## Benefits
- Single place to evolve CPU scheduling strategy.
- Easier to add telemetry/budgeting/throttling later.
- Smooth path to future backend split (CPU fallback vs GPU pipeline).
