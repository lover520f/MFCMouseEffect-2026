# Phase 32: Core System Folderization

## Background
After `Core/Config`, `Core/Control`, and `Core/Overlay` were established, platform/system-oriented units still sat at the `Core` root:
- GDI+ bootstrap
- global input hooks
- GPU runtime probing
- VM foreground detection

These are low-level OS/runtime concerns and should be isolated from orchestration and overlay domains.

## Goal
- Move platform/system concerns into an explicit package.
- Keep control/overlay code focused on orchestration and rendering behavior.
- Standardize include paths after move.

## Changes
1. Moved system files into `MouseFx/Core/System/`:
   - `GdiPlusSession.h`
   - `GlobalMouseHook.cpp/.h`
   - `GpuProbeHelper.cpp/.h`
   - `VmForegroundDetector.h`
2. Updated include call-sites:
   - `MouseFx/Core/System/GdiPlusSession.h`
   - `MouseFx/Core/System/GlobalMouseHook.h`
   - `MouseFx/Core/System/GpuProbeHelper.h`
   - `MouseFx/Core/System/VmForegroundDetector.h`
3. Fixed moved-source include stability:
   - `GlobalMouseHook.cpp` now includes `MouseFx/Core/MouseFxMessages.h` via stable root path.
4. Updated project metadata:
   - `MFCMouseEffect.vcxproj` retargeted moved system headers/sources.
   - `MFCMouseEffect.vcxproj.filters` retargeted moved system entries.

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 error`
  - `1 warning` (`HoldQuantumHaloGpuV2DirectRuntime.h` existing codepage warning, unrelated to this phase)

## Impact
- `Core` directory now has clearer subdomains:
  - `Core/Config`
  - `Core/Control`
  - `Core/Overlay`
  - `Core/System`
- Further cleanup can focus on remaining cross-cutting utility/protocol files (`JsonLite`, `MouseFxMessages`, `ConfigPathResolver`) with lower coupling risk.
