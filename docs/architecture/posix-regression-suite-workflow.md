# POSIX Regression Suite Workflow

## Priority-1 Command
Run this as the default POSIX regression entry:

```bash
./tools/platform/regression/run-posix-regression-suite.sh --platform auto
```

## What It Runs
1. Scaffold regression:
   - `run-posix-scaffold-regression.sh`
   - includes macOS tray `Settings` action + launcher URL call-path smoke assertion.
2. Core-lane smoke regression:
   - `run-posix-core-smoke.sh`
3. Core automation HTTP contracts:
   - `run-posix-core-automation-contract-regression.sh` (macOS-only, skipped on other platforms)
4. macOS automation injection selfcheck:
   - `run-macos-automation-injection-selfcheck.sh --skip-build --dry-run` (macOS host only, skipped on non-mac hosts)
5. macOS WASM runtime selfcheck:
   - `run-macos-wasm-runtime-selfcheck.sh --skip-build` (macOS host only, skipped on non-mac hosts)
6. Linux compile gate:
   - `run-posix-linux-compile-gate.sh`
   - default includes both `MFX_ENABLE_POSIX_CORE_RUNTIME=OFF/ON` compile lanes.
7. WebUI automation platform semantic tests:
   - `pnpm --dir MFCMouseEffect/WebUIWorkspace run test:automation-platform`

Before phases start, suite tries to terminate stale `mfx_entry_posix_host` processes to avoid single-instance/port contention from previous manual runs.

## Core Options
```bash
./tools/platform/regression/run-posix-regression-suite.sh \
  --platform auto \
  --scaffold-build-dir /tmp/mfx-platform-macos-build \
  --core-build-dir /tmp/mfx-platform-macos-core-build \
  --core-automation-build-dir /tmp/mfx-platform-macos-core-automation-build \
  --linux-build-dir /tmp/mfx-platform-linux-build \
  --jobs 12
```

Skip phases when needed:

```bash
./tools/platform/regression/run-posix-regression-suite.sh --skip-scaffold
./tools/platform/regression/run-posix-regression-suite.sh --skip-core-smoke
./tools/platform/regression/run-posix-regression-suite.sh --skip-core-automation
./tools/platform/regression/run-posix-regression-suite.sh --skip-macos-automation-injection-selfcheck
./tools/platform/regression/run-posix-regression-suite.sh --skip-macos-wasm-selfcheck
./tools/platform/regression/run-posix-regression-suite.sh --skip-linux-gate
./tools/platform/regression/run-posix-regression-suite.sh --linux-skip-core-runtime
./tools/platform/regression/run-posix-regression-suite.sh --skip-automation-test
```

## Packaging Layout (Keep This Boundary)
- Suite orchestrator:
  - `tools/platform/regression/run-posix-regression-suite.sh`
- Phase scripts (single-purpose):
  - `tools/platform/regression/run-posix-scaffold-regression.sh`
  - `tools/platform/regression/run-posix-core-smoke.sh`
  - `tools/platform/regression/run-posix-core-automation-contract-regression.sh`
  - `tools/platform/regression/run-posix-linux-compile-gate.sh`

## Extension Rule
1. Add new checks as phase-level scripts first.
2. Keep suite script orchestration-only.
3. Update this doc and related `docs/refactoring/*` in the same commit.
