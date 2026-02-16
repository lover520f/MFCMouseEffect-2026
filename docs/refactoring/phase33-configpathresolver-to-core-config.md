# Phase 33: Move ConfigPathResolver Into Core/Config

## Background
`ConfigPathResolver` was still placed at `MouseFx/Core` root, while its responsibility is fully configuration-domain related (resolving config location and associated paths).

After prior `Core/Config` extraction, this file remained as a boundary leak.

## Goal
- Keep config-domain primitives under `Core/Config`.
- Reduce `Core` root flatness and improve discoverability.

## Changes
1. Moved files:
   - `MouseFx/Core/ConfigPathResolver.cpp`
   - `MouseFx/Core/ConfigPathResolver.h`
   to:
   - `MouseFx/Core/Config/ConfigPathResolver.cpp`
   - `MouseFx/Core/Config/ConfigPathResolver.h`
2. Updated include call-sites to:
   - `MouseFx/Core/Config/ConfigPathResolver.h`
3. Updated project compile/include entries in:
   - `MFCMouseEffect.vcxproj`

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 error`
  - Existing unrelated `C4819` warning in `HoldQuantumHaloGpuV2DirectRuntime.h` remains.

## Impact
- `Core/Config` now owns path-resolution primitives used by config consumers.
- `Core` root continues to shrink toward true cross-domain/shared infrastructure only.
