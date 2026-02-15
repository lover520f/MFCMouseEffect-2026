# Phase 11: CommandHandler Encoding Cleanup

## Summary

Removed a non-ASCII character in `CommandHandler.cpp` header comment to eliminate persistent `C4819` source-encoding warning under current Windows code page builds.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/CommandHandler.cpp` | Replaced em dash (`—`) with ASCII hyphen (`-`) in file header comment |

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 warning`, `0 error`
