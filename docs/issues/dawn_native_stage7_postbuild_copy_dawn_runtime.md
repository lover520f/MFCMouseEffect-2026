# Dawn Native Stage 7: PostBuild Copy Dawn Runtime DLLs to Output

Date: 2026-02-13

## Problem
VS run logs still showed no `webgpu_dawn.dll` loaded in process, causing fallback on `hold_neon3d_gpu_v2`.

## Root Cause
Repository had Dawn runtime binaries under `MFCMouseEffect/Runtime/Dawn`, but project output (`x64/Release`) did not always contain `webgpu_dawn.dll`.

## Fix
Updated `MFCMouseEffect.vcxproj` post-build for `Debug|x64` and `Release|x64`:
1. keep existing WebUI copy
2. copy `Runtime/Dawn/webgpu_dawn.dll` to `$(OutDir)`
3. copy `Runtime/Dawn/d3dcompiler_47.dll` to `$(OutDir)`

## Validation
- Build should always place Dawn runtime DLLs beside `MFCMouseEffect.exe`.
- Runtime probe can hit primary path `<exe_dir>/webgpu_dawn.dll` deterministically.
