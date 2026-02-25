# Phase 54h - Linux Dual-Lane Compile Gate

## Background
- Linux compile gate previously validated one lane only (`MFX_ENABLE_POSIX_CORE_RUNTIME=OFF`).
- Recent mac-first changes increasingly touch core lane code paths.
- Linux contract goal is compile-level + contract-level follow; single-lane compile checks were no longer sufficient.

## Decision
- Keep Linux gate script as single entry point.
- Expand default gate behavior to compile both runtime lanes:
  - default lane (`MFX_ENABLE_POSIX_CORE_RUNTIME=OFF`)
  - core-runtime lane (`MFX_ENABLE_POSIX_CORE_RUNTIME=ON`)
- Preserve a fast local override for developers:
  - `--skip-core-runtime`

## Code Changes
1. Linux gate implementation expansion
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/linux_gate.sh`
- Added per-lane helper `_mfx_run_linux_compile_mode(...)`.
- `mfx_run_linux_compile_gate(...)` now builds:
  - `build_dir` (default lane, core runtime OFF)
  - `build_dir-core-runtime` (core lane, core runtime ON)

2. Entry script flags
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-linux-compile-gate.sh`
- Added flags:
  - `--skip-core-runtime`
  - `--include-core-runtime` (default behavior)

3. Workflow docs
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`
- Added dual-lane default and skip-mode usage.

## Behavior Compatibility
- Existing command remains valid:
  - `./tools/platform/regression/run-posix-linux-compile-gate.sh`
- CI/local gate coverage is stronger by default.
- No runtime semantics change.

## Functional Ownership
- Category: `Linux 跟随（编译级 + 契约级）`
- Coverage: dual-lane compile contracts for Linux package targets.

## Verification
1. `./tools/platform/regression/run-posix-linux-compile-gate.sh --build-dir /tmp/mfx-platform-linux-build`
- Result: passed.

2. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.

3. `./tools/docs/doc-hygiene-check.sh --strict`
- Result: passed.
