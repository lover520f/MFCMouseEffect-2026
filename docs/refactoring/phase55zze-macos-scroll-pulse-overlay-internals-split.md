# Phase 55zze: macOS Scroll Pulse Overlay Internals Split

## Capability
- Effect

## Why
- `MacosScrollPulseOverlayRenderer.mm` still carried two independent responsibilities:
  - overlay style generation (stroke/fill colors, direction arrow path)
  - transient window lifecycle registry (register/take/close-all)
- Keeping both in renderer increased local coupling and made follow-up behavior changes harder to review.

## Scope
- Keep visible scroll pulse behavior unchanged.
- Split style logic and window-registry logic into dedicated modules.
- Keep renderer focused on overlay construction/animation orchestration.

## Code Changes

### 1) New style module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayStyle.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayStyle.mm`
- Owns:
  - stroke/fill color selection by axis + delta
  - directional arrow path generation

### 2) New window-registry module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseWindowRegistry.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseWindowRegistry.mm`
- Owns:
  - transient overlay window handle set
  - register/take semantics
  - close-all drain on shutdown

### 3) Renderer simplification
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRenderer.mm`
- Keeps:
  - main-thread dispatch wrappers
  - pulse strength/animation timing
  - overlay composition and close-after scheduling

### 4) Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API/schema/behavior contract changes.
- Scroll pulse effect rendering semantics are unchanged.
