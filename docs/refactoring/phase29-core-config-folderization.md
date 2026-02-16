# Phase 29: Core Config Folderization

## Background
`MouseFx/Core` had become increasingly flat, with configuration model, JSON codec, and runtime controller logic mixed in one directory level.
Even after multi-file refactors, discoverability and ownership boundaries remained weak.

## Changes

### 1) Introduced config domain subfolder
- Created `MFCMouseEffect/MouseFx/Core/Config/`
- Moved the complete `EffectConfig*` family into this folder:
  - config model and persistence (`EffectConfig.*`, `EffectConfigInternal.h/.cpp`)
  - JSON codec and internal helpers
  - JSON key catalog headers
  - effect color mapping helpers

### 2) Updated include paths at integration points
- Core internal includes:
  - `AppController.h`
  - `EffectFactory.h`
  - `InputIndicatorOverlay.h`
- External dependencies now reference:
  - `MouseFx/Core/Config/EffectConfig.h`
  (updated in effects/layers/interfaces/windows/server integration files)

### 3) Updated project compile/include entries
- Updated `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - all moved files now point to `MouseFx\Core\Config\...`

## Why this helps
- Makes `Core` structure explicitly domain-oriented.
- Separates config domain from controller/router/hook/runtime concerns.
- Reduces cognitive load when navigating or reviewing config-related changes.

## Compatibility
- No behavior change intended.
- JSON schema and runtime config semantics are unchanged.
- Include path changes are purely structural.

## Build Verification

Command:

```powershell
C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

Result:
- Build succeeded
- `0` warnings, `0` errors
