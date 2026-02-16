# Phase 30: Core Control Folderization

## Background
`MouseFx/Core` had started to flatten again after config-domain extraction.  
Control-plane classes (`AppController`, command routing, dispatch routing, stdin IPC control) were still mixed with platform/helpers in the same level.

## Goal
- Keep `Core` as a composition root, not a dumping ground.
- Group control-plane responsibilities into a dedicated package boundary.
- Remove fragile same-directory include assumptions introduced after file moves.

## Changes
1. Moved control-plane units into `MouseFx/Core/Control/`:
   - `AppController.*`
   - `CommandHandler.*`
   - `DispatchRouter.*`
   - `IpcController.*`
2. Updated include call-sites to new public paths:
   - `MouseFx/Core/Control/AppController.h`
   - `MouseFx/Core/Control/IpcController.h`
3. Hardened internal includes in control units to stable root-relative paths:
   - `MouseFx/Core/ConfigPathResolver.h`
   - `MouseFx/Core/EffectFactory.h`
   - `MouseFx/Core/OverlayHostService.h`
   - `MouseFx/Core/JsonLite.h`
   - `MouseFx/Core/MouseFxMessages.h`
   - `MouseFx/Core/GpuProbeHelper.h`
   - `MouseFx/Core/VmForegroundDetector.h`
   - `MouseFx/Core/GdiPlusSession.h`
   - `MouseFx/Core/GlobalMouseHook.h`
   - `MouseFx/Core/Config/EffectConfig.h`
4. Updated project metadata:
   - `MFCMouseEffect.vcxproj` compile/include entries now point to `Core/Control`.
   - Fixed stale header path entry `MouseFx\\MouseFxMessages.h` -> `MouseFx\\Core\\MouseFxMessages.h`.
   - `MFCMouseEffect.vcxproj.filters` updated for moved `AppController` entries.

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 warning / 0 error`

## Impact
- `Core` package responsibilities are clearer:
  - `Core/Config`: config domain
  - `Core/Control`: lifecycle/command/dispatch/ipc control domain
  - `Core/*`: remaining infra/services to be cleaned in next phases
- Follow-up refactoring can now split infra (`hook/session/vm`) and service (`overlay/factory/probe`) with lower risk.
