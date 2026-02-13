# Dawn Native Stage 3: Runtime DLL Probe Wiring

Date: 2026-02-13

## Goal
Wire real Dawn runtime binary probing/loading before applying CPU fallback for `hold_neon3d_gpu_v2`.

## Changes
- Restored Dawn runtime artifacts into repository:
  - `MFCMouseEffect/Runtime/Dawn/webgpu_dawn.dll`
  - `MFCMouseEffect/Runtime/Dawn/d3dcompiler_47.dll`
  - `MFCMouseEffect/Runtime/Dawn/README.md`
- Added runtime probe in `AppController`:
  - Attempts `LoadLibraryW` from:
    1. `<exe_dir>/webgpu_dawn.dll`
    2. `<exe_dir>/Runtime/Dawn/webgpu_dawn.dll`
  - Probe result drives fallback reason:
    - `dawn_runtime_binary_missing_or_load_failed`
    - `dawn_runtime_ready_but_backend_not_integrated`

## Why
- This makes the Dawn route fallback reason deterministic and tied to actual runtime loadability, not just static assumptions.
- It prepares the next step where a loadable runtime can proceed into real Dawn backend activation.

## Validation
- Build target `Release|x64` passes.
- Selecting `hold_neon3d_gpu_v2` writes meaningful route reason into local diag snapshot.
