# AI Context Map

Generated: 2026-03-13T09:58:33.696Z

## Goal
Load minimal docs by task keyword while keeping AGENTS + current context as mandatory baseline.

## Mandatory First Read
- 1. `AGENTS.md` (1500 tok)
- 2. `docs/agent-context/current.md` (1100 tok)
- 3. `docs/refactoring/phase-roadmap-macos-m1-status.md` (1100 tok)

## Topic Routes (Top Candidates)
### automation
- `docs/agent-context/current.md` (P1, 1100 tok)
- `docs/agent-context/p2-capability-index.md` (P2, 636 tok)
- `docs/architecture/agent-doc-governance.md` (P2, 750 tok)
- `docs/architecture/posix-core-automation-contract-workflow.md` (P2, 750 tok)

### wasm
- `docs/architecture/custom-effects-wasm-route.md` (P2, 750 tok)
- `docs/architecture/custom-effects-wasm-route.zh-CN.md` (P2, 750 tok)
- `docs/architecture/wasm-plugin-abi-v3-design.md` (P2, 750 tok)
- `docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md` (P2, 750 tok)

### workflow
- `AGENTS.md` (P0, 1500 tok)
- `docs/architecture/posix-core-lane-smoke-workflow.md` (P2, 435 tok)
- `docs/architecture/posix-linux-compile-gate-workflow.md` (P2, 490 tok)

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
- `docs/refactoring/phase-roadmap-macos-m1-status.md` -> ~5868 tok
- `docs/automation/gesture-matching.md` -> ~3436 tok
- `docs/agent-context/current.md` -> ~2624 tok
- `docs/architecture/wasm-plugin-abi-v3-design.md` -> ~2078 tok
- `docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md` -> ~1996 tok
- `AGENTS.md` -> ~1918 tok

## Notes
- Index is machine-readable: `docs/.ai/context-index.json`.
- Route output uses token budget + keyword scoring + priority gating.
- `watch` rebuilds index on AGENTS/docs markdown changes.
