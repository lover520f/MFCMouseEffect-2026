# Phase 56zn - macOS Style Color Decouple and ObjC++ Surface Prune

## What Changed
- Decoupled `click/trail/scroll/hover` style modules from `NSColor/AppKit` contracts:
  - Removed style-level `NSColor*` APIs from:
    - `MacosClickPulseOverlayStyle.*`
    - `MacosTrailPulseOverlayStyle.*`
    - `MacosScrollPulseOverlayStyle.*`
    - `MacosHoverPulseOverlayStyle.*`
  - Kept only style concerns needed by shared render math/type helpers (normalize/path construction).
- Moved scroll color realization to renderer support (`MacosScrollPulseOverlayRendererSupport.cpp`) using ARGB input from command fields, preserving existing behavior.
- Updated scroll support API signatures to consume resolved ARGB directly and removed redundant profile-based color lookup path.
- Removed the four style `.cpp` files from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Why
- These style units were mostly compute/normalization logic and did not require Objective-C runtime semantics.
- Keeping them in ObjC++ mode inflated residual ObjC++ surface and blurred migration progress.
- Color decisions are already resolved in render command fields; duplicating profile lookups in style layer was unnecessary.

## Validation
- Syntax probe (`-x c++ -fsyntax-only`) passed for:
  - `MacosClickPulseOverlayStyle.cpp`
  - `MacosTrailPulseOverlayStyle.cpp`
  - `MacosScrollPulseOverlayStyle.cpp`
  - `MacosHoverPulseOverlayStyle.cpp`
- Build:
  - `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
  - `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- Regression:
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
  - `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## Result
- All gates passed.
- ObjC++ allowlist count reduced from `29` to `25` with no user-visible behavior changes.
