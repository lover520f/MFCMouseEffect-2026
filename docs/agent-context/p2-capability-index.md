# P2 Capability Index (Task-Scoped Reads)

## Purpose
This file is a P2 routing map.  
Use it when P1 (`current.md`) is not enough and you need targeted deep context.

## Routing Rules
- Always read P0/P1 first:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
- Then pick only one or two P2 docs by task keyword.
- Do not bulk-open all P2 docs.

## Automation / Gesture
- Gesture matching logic, thresholds, and regression notes:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/gesture-matching.md`
- POSIX automation contract workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`

## WASM / Plugins
- Core wasm route and architecture:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.zh-CN.md`
- ABI details:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md`

## Regression / Workflow
- Unified regression workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- Core lane smoke workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- Linux compile gate:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`

## Archive (Only When Needed)
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`

## AI Router Commands
```bash
./tools/docs/ai-context.sh index
./tools/docs/ai-context.sh route --task "automation gesture debug"
./tools/docs/ai-context.sh check --strict
./tools/docs/ai-context.sh watch
```
