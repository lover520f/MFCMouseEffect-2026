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
- Gesture debug UI rendering and preview behavior:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/gesture-debug-ui-notes.md`
- Automation mapping behavior details:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/automation-mapping-notes.md`
- POSIX automation contract workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`

## WASM / Plugins
- Core wasm route and architecture:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.zh-CN.md`
- ABI details:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md`

## Server / WebSettings
- Server layer map and include boundaries:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/server-structure.md`

## Mouse Companion 3D
- 插件化重建主路线（当前最高优先级，按 Native Plugin 优先、WASM 可选适配执行）:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-plugin-landing-roadmap.zh-CN.md`
- 后端重置契约（当前最高优先级，说明已清空旧后端实现并保留前端/配置兼容面）:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-backend-reset-contract.zh-CN.md`
- 3D companion runtime blueprint and phased architecture (canonical `glb`, non-builtin animation pipeline):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-3d-runtime-blueprint.zh-CN.md`
- Action clip asset contract and JSON schema (external skeleton action reuse):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-action-clip-contract.zh-CN.md`
- Appearance customization contract (profile JSON + accessory/material override):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-appearance-contract.zh-CN.md`
- Model import pipeline contract (converter abstraction + sidecar/default pipeline):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-model-import-pipeline-contract.zh-CN.md`
- Position mode contract (`fixed_bottom_left` / `follow`, macOS left-bottom anchor path):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-position-mode-contract.zh-CN.md`
- Click parity contract (tauri-aligned click gate + limb procedural linkage):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/mouse-companion-click-parity-tauri-contract.zh-CN.md`

## Regression / Workflow
- Unified regression workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- Core lane smoke workflow:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- Linux compile gate:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`

## Ops / Manual
- Manual command cheat sheet:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/ops/manual-commands.md`

## Archive (Only When Needed)
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`

## AI Router Commands
```bash
./tools/docs/ai-context.sh index
./tools/docs/ai-context.sh route --task "automation gesture debug"
./tools/docs/ai-context.sh check --strict
./tools/docs/ai-context.sh watch
```
