# Phase 31: Core Overlay Folderization

## Background
After extracting `Core/Config` and `Core/Control`, overlay-related infrastructure was still flattened in `MouseFx/Core`:
- `InputIndicatorOverlay.*`
- `OverlayCoordSpace.*`
- `OverlayHostService.*`

These files form one cohesive domain (overlay window lifecycle, coordinate mapping, and layer host service), but they were colocated with unrelated responsibilities.

## Goal
- Group overlay domain into a dedicated package boundary.
- Reduce `Core` flatness and improve discoverability.
- Keep include paths stable after move to avoid fragile relative includes.

## Changes
1. Moved files into `MouseFx/Core/Overlay/`:
   - `InputIndicatorOverlay.cpp/.h`
   - `OverlayCoordSpace.cpp/.h`
   - `OverlayHostService.cpp/.h`
2. Updated include call-sites across effects/layers/renderers/windows/control:
   - `MouseFx/Core/Overlay/InputIndicatorOverlay.h`
   - `MouseFx/Core/Overlay/OverlayCoordSpace.h`
   - `MouseFx/Core/Overlay/OverlayHostService.h`
3. Fixed moved-header include stability:
   - `InputIndicatorOverlay.h` now references
     - `MouseFx/Core/Config/EffectConfig.h`
     - `MouseFx/Core/GlobalMouseHook.h`
4. Updated project metadata:
   - `MFCMouseEffect.vcxproj` compile/include entries retargeted to `Core/Overlay`.
   - `MFCMouseEffect.vcxproj.filters` entries retargeted to `Core/Overlay`.

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 warning / 0 error`

## Impact
- `Core` package boundaries become clearer:
  - `Core/Config`
  - `Core/Control`
  - `Core/Overlay`
- Overlay-domain changes can now evolve with lower coupling to control/config/platform code.
