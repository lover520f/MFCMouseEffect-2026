# Phase 34: Core Protocol Folderization

## Background
`JsonLite` and `MouseFxMessages` were still placed at `MouseFx/Core` root.

Both files belong to command/message protocol concerns:
- `JsonLite`: lightweight command JSON field extraction.
- `MouseFxMessages`: shared Win32 dispatch message contracts.

Keeping them in root reduced boundary clarity between domain orchestration and protocol definitions.

## Goal
- Extract protocol primitives into a dedicated package.
- Keep control/system modules dependent on explicit protocol boundary.

## Changes
1. Moved protocol files to `MouseFx/Core/Protocol/`:
   - `JsonLite.cpp/.h`
   - `MouseFxMessages.h`
2. Updated include call-sites:
   - `MouseFx/Core/Protocol/JsonLite.h`
   - `MouseFx/Core/Protocol/MouseFxMessages.h`
3. Updated project metadata:
   - `MFCMouseEffect.vcxproj` include/compile entries retargeted.
   - `MFCMouseEffect.vcxproj.filters` message header path retargeted.

## Validation
- Build command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - `0 warning / 0 error`

## Impact
- `Core` now exposes clearer subdomains:
  - `Config` / `Control` / `Overlay` / `System` / `Protocol`
- Control and hook pipelines now consume protocol contracts through a stable package path.
