# phase56g: scaffold regression lane pin (core-runtime OFF)

## Why
- Scaffold regression and core-lane regression may reuse the same build directory.
- When cache keeps `MFX_ENABLE_POSIX_CORE_RUNTIME=ON`, scaffold smoke checks can execute against wrong lane.

## Change
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-scaffold-regression.sh`:
  - force terminate stale `mfx_entry_posix_host` before scaffold checks;
  - configure scaffold build with explicit `-DMFX_ENABLE_POSIX_CORE_RUNTIME=OFF`.

## Validation
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`
- Result: passed.
