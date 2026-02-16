# Phase 35: Move EffectFactory Into Core/Control

## Background
After previous folderization phases, `MouseFx/Core` root only contained `EffectFactory`.

`EffectFactory` is consumed by `AppController` and is part of control-plane orchestration (category/type -> effect instance), rather than a cross-domain root primitive.

## Goal
- Co-locate effect orchestration with control domain.
- Eliminate remaining root-level implementation files under `Core`.

## Changes
1. Moved files:
   - `MouseFx/Core/EffectFactory.cpp/.h`
   to:
   - `MouseFx/Core/Control/EffectFactory.cpp/.h`
2. Updated include call-sites:
   - `MouseFx/Core/Control/EffectFactory.h`
3. Updated moved header dependency to stable config path:
   - `MouseFx/Core/Config/EffectConfig.h`
4. Updated project metadata:
   - `MFCMouseEffect.vcxproj` include/compile entries retargeted.

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 warning / 0 error`

## Impact
- `Core` root no longer stores active domain implementation files.
- Control-plane orchestration artifacts (`AppController`, `CommandHandler`, `DispatchRouter`, `IpcController`, `EffectFactory`) are now colocated.
