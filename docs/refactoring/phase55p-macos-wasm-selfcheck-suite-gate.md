# Phase 55p: macOS WASM Selfcheck Suite Gate

## Issue Classification
- Verdict: `Process debt`.
- Problem: macOS WASM runtime selfcheck already existed but lived outside the unified POSIX suite, so invoke/render/fallback acceptance could be skipped unintentionally.

## Goal
1. Promote macOS WASM runtime selfcheck to a first-class suite phase.
2. Keep one-command suite behavior stable while allowing explicit skip for triage.
3. Reduce manual acceptance drift for `load -> enable -> dispatch -> fallback`.

## Implementation
- Unified suite integration:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-regression-suite.sh`
  - add phase:
    - runs `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`
    - reuses core build dir (`--core-automation-build-dir` / `--core-build-dir`) when provided.
  - add skip option:
    - `--skip-macos-wasm-selfcheck`
  - run condition:
    - macOS host only; non-mac hosts report explicit skip (no failure).

## Validation
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-core-build`
- `./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build --build-dir /tmp/mfx-platform-macos-core-build`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Closure
- macOS WASM invoke/render/fallback selfcheck is now part of default suite execution, reducing manual-only regression risk while preserving a targeted skip switch for debugging.
