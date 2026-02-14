# Dawn Native Stage 6: Add Repository Runtime Probe Path

Date: 2026-02-13

## Problem
Runtime probe reported:
- `dawn_runtime_binary_missing_or_load_failed`
even though Dawn DLLs existed in repository runtime folder.

## Root Cause
Probe only checked:
1. `<exe_dir>/webgpu_dawn.dll`
2. `<exe_dir>/Runtime/Dawn/webgpu_dawn.dll`

But current dev layout stores binaries at:
- `<repo_root>/MFCMouseEffect/Runtime/Dawn/webgpu_dawn.dll`

## Fix
Added a third probe path:
- `<repo_root>/MFCMouseEffect/Runtime/Dawn/webgpu_dawn.dll`

with success reason:
- `dawn_runtime_loaded_from_repo_runtime_dir`

## Validation
- Build `Release|x64` passes.
- Selecting `hold_neon3d_gpu_v2` should no longer fallback due to missing binary in this layout.
