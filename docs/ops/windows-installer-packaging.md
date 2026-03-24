# Windows Installer Packaging

## Scope
- Windows installer path uses Inno Setup script:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/Install/MFCMouseEffect.iss`
- Preferred user entrypoint:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/./mfx package`
- Runtime source payload is taken from:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/x64/Release`

## Current Policy
- Deliver runtime-focused payload only:
  - `MFCMouseEffect.exe`
  - optional persisted `config.json`
  - `webui/`
  - runtime DLLs required by the current Windows lane
- Windows package build now accepts an explicit GPU toggle:
  - `./mfx package --gpu`
  - `./mfx package --no-gpu`
  - `--no-gpu` forwards `MfxEnableWindowsGpuEffects=false`, excludes Windows GPU hold compile units from the Release build, and removes `webgpu_dawn.dll` from the installer payload
- Do not copy optional repo docs like `README.md` or `LICENSE` into the install directory.
- Do not expose installer-side launch-at-startup configuration.
  - Reason: launch-at-startup is now owned by the Web settings + native platform implementation (`PlatformLaunchAtStartup`) and should not have a second policy surface in the installer.

## Artifact Naming
- Current setup artifact name:
  - `MFCMouseEffect-windows-x64-setup-<version>.exe`
- Output root stays:
  - `Install/windows/`

## Validation
1. Build Windows release:
  - `./mfx package`
  - or `./mfx package-no-build` when reusing the current `Release|x64` output
  - GPU-selectable examples:
    - `./mfx package --gpu`
    - `./mfx package --no-gpu`
2. Compile Inno Setup script:
   - internal implementation still uses Inno Setup, but `ISCC.exe` is no longer the user-facing workflow entry
3. Verify installer contents:
   - install directory contains runtime payload only
   - `--gpu`: includes `webgpu_dawn.dll`
   - `--no-gpu`: must not include `webgpu_dawn.dll`
   - installer UI no longer shows a startup task
4. Verify startup policy:
   - use Web settings `launch_at_startup`
   - confirm native platform registration is applied/removed correctly

## Known Boundary
- `--gpu` package size is still dominated by:
  - `webgpu_dawn.dll`
- `--no-gpu` can meaningfully shrink the installer, but it also removes the current Windows GPU hold route family by build contract rather than only by runtime fallback.
