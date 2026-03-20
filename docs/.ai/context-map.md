# AI Context Map

Generated: 2026-03-20T10:32:37.220Z

## Goal
Load minimal docs by task keyword while keeping AGENTS + current context as mandatory baseline.

## Mandatory First Read
- 1. `AGENTS.md` (1500 tok)
- 2. `docs/agent-context/current.md` (1100 tok)
- 3. `docs/refactoring/phase-roadmap-macos-m1-status.md` (1100 tok)

## Topic Routes (Top Candidates)
### automation
- `docs/agent-context/current.md` (P1, 1100 tok)
- `docs/agent-context/p2-capability-index.md` (P2, 750 tok)
- `docs/architecture/agent-doc-governance.md` (P2, 750 tok)
- `docs/architecture/posix-core-automation-contract-workflow.md` (P2, 750 tok)

### wasm
- `docs/architecture/custom-effects-wasm-route.md` (P2, 750 tok)
- `docs/architecture/custom-effects-wasm-route.zh-CN.md` (P2, 750 tok)
- `docs/architecture/mouse-companion-backend-reset-contract.zh-CN.md` (P2, 750 tok)
- `docs/architecture/mouse-companion-plugin-landing-roadmap.zh-CN.md` (P2, 750 tok)

### effects
- `docs/architecture/mouse-companion-action-clip-contract.zh-CN.md` (P2, 750 tok)
- `docs/architecture/mouse-companion-procedural-effect-profile-contract.zh-CN.md` (P2, 750 tok)

### input-indicator
- `docs/architecture/mouse-companion-position-mode-contract.zh-CN.md` (P2, 750 tok)

### workflow
- `AGENTS.md` (P0, 1500 tok)
- `docs/architecture/posix-core-lane-smoke-workflow.md` (P2, 435 tok)
- `docs/architecture/posix-linux-compile-gate-workflow.md` (P2, 490 tok)

### general
- `docs/architecture/mouse-companion-3d-runtime-blueprint.zh-CN.md` (P2, 750 tok)
- `docs/architecture/mouse-companion-appearance-contract.zh-CN.md` (P2, 645 tok)
- `docs/architecture/mouse-companion-click-parity-tauri-contract.zh-CN.md` (P2, 750 tok)
- `docs/architecture/mouse-companion-model-import-pipeline-contract.zh-CN.md` (P2, 750 tok)

## Commands
```bash
./tools/docs/ai-context.sh index
./tools/docs/ai-context.sh route --task "automation gesture debug"
./tools/docs/ai-context.sh check --strict
./tools/docs/ai-context.sh watch
```

## Largest Docs (Trim Candidates)
- `docs/architecture/custom-effects-wasm-route.zh-CN.md` -> ~6872 tok
- `docs/architecture/custom-effects-wasm-route.md` -> ~6707 tok
- `docs/agent-context/current.md` -> ~6550 tok
- `docs/refactoring/phase-roadmap-macos-m1-status.md` -> ~5868 tok
- `docs/automation/gesture-matching.md` -> ~4462 tok
- `docs/architecture/mouse-companion-3d-runtime-blueprint.zh-CN.md` -> ~3937 tok
- `docs/architecture/mouse-companion-plugin-landing-roadmap.zh-CN.md` -> ~2561 tok
- `docs/architecture/wasm-plugin-abi-v3-design.md` -> ~2078 tok

## Notes
- Index is machine-readable: `docs/.ai/context-index.json`.
- Route output uses token budget + keyword scoring + priority gating.
- `watch` rebuilds index on AGENTS/docs markdown changes.
