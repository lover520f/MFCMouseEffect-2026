# Windows Installer Packaging

## Scope
- Preferred user entrypoints now stay inside `mfx`:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/./mfx build`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/./mfx package`
- Windows installer path uses Inno Setup script:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/Install/MFCMouseEffect.iss`
- Runtime source payload is taken from:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/x64/Release`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/x64/Shipping` when `--shipping` is used

## Current Policy
- Deliver runtime-focused payload only:
  - `MFCMouseEffect.exe`
  - optional persisted `config.json`
  - `webui/`
  - runtime DLLs required by the current Windows lane
- Windows package build now accepts an explicit GPU toggle:
  - `./mfx build`
  - `./mfx build --gpu`
  - `./mfx build --shipping`
  - `./mfx package`
  - `./mfx package --shipping`
  - `./mfx package --gpu`
  - `./mfx package --no-gpu`
  - default is now `--no-gpu`
  - `./mfx package` now reuses the same Windows build contract as `./mfx build`
  - `--shipping` now forwards `BuildConfiguration=Shipping` into Inno Setup and packages from `x64/Shipping`
  - `--no-gpu` forwards `MfxEnableWindowsGpuEffects=false`, excludes Windows GPU hold compile units from the Release build, and removes `webgpu_dawn.dll` from the installer payload
- Do not copy optional repo docs like `README.md` or `LICENSE` into the install directory.
- Do not expose installer-side launch-at-startup configuration.
  - Reason: launch-at-startup is now owned by the Web settings + native platform implementation (`PlatformLaunchAtStartup`) and should not have a second policy surface in the installer.

## Artifact Naming
- Current setup artifact name:
  - `MFCMouseEffect-windows-x64-setup-<version>.exe`
- Current shipping setup artifact name:
  - `MFCMouseEffect-windows-x64-shipping-setup-<version>.exe`
- GPU package setup artifact name:
  - `MFCMouseEffect-windows-x64-gpu-setup-<version>.exe`
- GPU shipping package setup artifact name:
  - `MFCMouseEffect-windows-x64-gpu-shipping-setup-<version>.exe`
- If `--package-name` is explicitly provided, it still overrides both defaults.
- Output root stays:
  - `Install/windows/`

## Validation
1. Build Windows release:
   - `./mfx build`
   - or directly `./mfx package`
     - default: no GPU
   - `./mfx build --gpu`
   - `./mfx build --shipping`
   - `./mfx package --shipping`
   - or `./mfx package-no-build` when reusing the current selected output
  - GPU-selectable examples:
    - `./mfx package --gpu`
    - `./mfx package --no-gpu`
    - `./mfx package --shipping --gpu`
2. Compile Inno Setup script:
   - internal implementation still uses Inno Setup, but `ISCC.exe` is no longer the user-facing workflow entry
3. Verify installer contents:
   - install directory contains runtime payload only
   - default / `--no-gpu`: must not include `webgpu_dawn.dll`
   - `--gpu`: includes `webgpu_dawn.dll`
   - installer UI no longer shows a startup task
4. Verify startup policy:
   - use Web settings `launch_at_startup`
   - confirm native platform registration is applied/removed correctly

## Size Notes
- The installer currently uses Inno Setup with `Compression=lzma2/ultra64` (size-first; slower to compile).
- Biggest size drivers are typically:
  - `webui/` bundle size
  - optional GPU lane payload (`webgpu_dawn.dll`)
- Quick size comparison workflow:
  1. Build two installers back-to-back:
     - `./mfx package --no-gpu`
     - `./mfx package --gpu`
  2. Compare the generated `Install/windows/*.exe` sizes.

## Known Boundary
- `--gpu` package size is still dominated by:
  - `webgpu_dawn.dll`
- `--no-gpu` can meaningfully shrink the installer, but it also removes the current Windows GPU hold route family by build contract rather than only by runtime fallback.
