# MFCMouseEffect 文档

语言： [English](README.md) | [中文](README.zh-CN.md)

## 目标
本索引保持精简，优先服务 AI-IDE 快速检索。禁止在这里继续堆长历史清单。

## 建议阅读顺序（Agent-First）
1. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
2. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
3. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
4. 仅按当前任务打开 1 篇目标文档（`refactoring` 或 `issues`）

## 文档分层
- `P0` 全局协作约束：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
- `P1` 当前上下文：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- `P2` 能力文档（按需打开）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/`
- `P3` 归档区：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`

## 稳定工作流文档
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/agent-doc-governance.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-scaffold-regression-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/trail-profiles-config.zh-CN.md`

## 定向检索命令
```bash
# 最近重构文档
rg --files docs/refactoring | sort | tail -n 30

# 按能力关键词查文档
rg -n "permission|automation|app_scope|effects|wasm" docs/refactoring docs/issues docs/architecture

# 文档体积治理
./tools/docs/doc-hygiene-check.sh --strict
```

## macOS 本地命令
```bash
./mfx start
./mfx fast
./mfx effects
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
./tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

## 维护规则
顶层索引只保留导航，不记录过程细节；细节放在对应能力文档。
