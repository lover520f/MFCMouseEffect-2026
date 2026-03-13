# Agent-First Documentation Governance

## Objective
Keep project docs high-signal, searchable, and token-efficient for AI-IDE and human review.

## Layering Model
- `P0` Global contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
- `P1` Active context:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- `P2` Capability docs:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
  - one targeted issue/refactoring doc per task.
- `P3` Archive/low-priority:
  - long process records and obsolete execution details.

## What belongs where
- Put in `P1`:
  - active milestones
  - current behavior contracts
  - regression gate commands
  - current risks and next slice
- Put in `P2`:
  - feature-specific implementation details
  - bug root cause and fix path
  - acceptance evidence
- Put in `P3`:
  - low-reuse process history
  - superseded plans and lengthy execution logs

## Token Budget Policy
- Keep these files compact:
  - `AGENTS.md` <= 260 lines (target)
  - `docs/agent-context/current.md` <= 220 lines (target)
  - `docs/README.md` <= 140 lines (target)
  - `docs/README.zh-CN.md` <= 140 lines (target)
- Avoid exhaustive lists in top-level index files.
- Prefer pointer-style indexes + targeted retrieval.

## Update Policy
For each code change that affects behavior/contracts:
1. Update the relevant capability doc (`P2`).
2. Update `P1` (`current.md` and/or roadmap) if active truth changed.
3. Update README indexes only for navigation-level changes.

## Maintenance Cadence
- Weekly maintenance:
  - run `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/docs/doc-hygiene-check.sh --strict`
  - remove stale low-value text from `P1` and top-level indexes
- Monthly maintenance:
  - move stale low-priority docs to archive area
  - keep high-value decisions/contracts in `P1/P2`

## Retrieval Strategy
1. Always read `AGENTS.md` and `docs/agent-context/current.md` first.
2. Use `rg` to locate one or two targeted docs only.
3. Do not bulk-open `/docs/issues` or `/docs/refactoring`.

## Suggested Commands
```bash
# Compact health check
./tools/docs/doc-hygiene-check.sh --strict

# Find latest phase docs
rg --files docs/refactoring | sort | tail -n 20

# Find target docs by keyword
rg -n "permission|wasm|automation|app_scope" docs/refactoring docs/issues
```

## Context Router (mandatory upkeep)
- Generate index artifacts after doc updates:
  - `./tools/docs/ai-context.sh index`
- Validate index freshness before commit/CI:
  - `./tools/docs/ai-context.sh check --strict`
- Optional line-limit hard gate:
  - `./tools/docs/ai-context.sh check --strict --enforce-line-limits`
- Build task-scoped minimal read set:
  - `./tools/docs/ai-context.sh route --task "<task keywords>"`
- Optional local realtime update while editing docs:
  - `./tools/docs/ai-context.sh watch`
- Optional pre-commit auto-refresh/install:
  - `./tools/docs/install-git-hook.sh`

Generated artifacts:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-index.json`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-map.md`
