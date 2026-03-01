# MFCMouseEffect Documentation

Language: [English](README.md) | [中文](README.zh-CN.md)

## Purpose
Compact AI-first index for fast navigation. This file is intentionally short and avoids historical long lists.

## Read Order (Agent-First)
1. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
2. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
3. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
4. One targeted capability doc in `docs/refactoring/` or `docs/issues/`

## Priority Layers
- `P0` global contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
- `P1` active context:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- `P2` capability docs:
  - targeted docs under `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/`
  - targeted docs under `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/`
- `P3` archive:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`

## Stable Workflow Docs
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/agent-doc-governance.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-scaffold-regression-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/trail-profiles-config.md`

## Targeted Retrieval
```bash
# latest refactoring docs
rg --files docs/refactoring | sort | tail -n 30

# find docs by capability
rg -n "permission|automation|app_scope|effects|wasm" docs/refactoring docs/issues docs/architecture

# doc hygiene gate
./tools/docs/doc-hygiene-check.sh --strict
```

## macOS Local Commands
```bash
./mfx run
./mfx run-no-build
./mfx run-no-build --seconds 30
./mfx effects
# backward-compatible aliases
./mfx start
./mfx fast
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
./tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

## Note
Do not add long historical phase lists back to this top-level index. Keep details in targeted docs.
