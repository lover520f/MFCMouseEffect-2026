# Phase 36: WebSettingsServer Multi-Unit Split

## Background
`WebSettingsServer.cpp` mixed multiple responsibilities in one file:
- server lifecycle boot/shutdown
- token verification + idle monitor thread
- API routing + static asset routing + exception mapping

This created a high-coupling hotspot and reduced readability for future changes.

## Goal
- Split `WebSettingsServer` implementation by responsibility.
- Keep behavior unchanged while reducing single-file complexity.

## Changes
1. Kept lifecycle/token bootstrap in:
   - `MouseFx/Server/WebSettingsServer.cpp`
2. Extracted request routing into:
   - `MouseFx/Server/WebSettingsServer.Routing.cpp`
   - `HandleRequest`
   - `HandleApiRoute`
   - `HandleStaticAssetRoute`
3. Extracted token + idle monitor logic into:
   - `MouseFx/Server/WebSettingsServer.TokenMonitor.cpp`
   - `IsTokenValid`
   - `RotateToken`
   - `Touch`
   - `StartMonitor`
   - `StopMonitor`
   - `StopAsync`
4. Updated class private declarations in:
   - `MouseFx/Server/WebSettingsServer.h`
5. Updated project metadata:
   - `MFCMouseEffect.vcxproj` add new compile units
   - `MFCMouseEffect.vcxproj.filters` add new compile units

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 warning / 0 error`

## Impact
- `WebSettingsServer` now has explicit internal boundaries:
  - lifecycle/bootstrap
  - request routing
  - token + monitor
- Future API route and idle policy adjustments can be made with lower regression risk.
