# C++17 Language Standard Unification

## Background
- The project already used `stdcpp17` in `Debug|x64`.
- Other configurations (`Debug|Win32`, `Release|Win32`, `Release|x64`) were not explicitly aligned.
- This could cause syntax/feature mismatch across build targets.

## Change
- Unified all configurations to C++17 by setting:
  - `<LanguageStandard>stdcpp17</LanguageStandard>`
- Updated file:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`

## Scope
- `Debug|Win32`
- `Release|Win32`
- `Release|x64`
- (`Debug|x64` already had C++17; kept unchanged)

## Additional
- Added `MouseFx/Compute/EffectComputeExecutor.h` into project include list for IDE visibility:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`
