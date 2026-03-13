# MFCMouseEffect 文档

语言： [English](README.md) | [中文](README.zh-CN.md)

## 目标
本索引保持精简，优先服务 AI-IDE 快速检索。禁止在这里继续堆长历史清单。

## 建议阅读顺序（Agent-First）
1. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
2. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
3. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
4. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
5. 仅按当前任务打开 1 篇目标文档

## 文档分层
- `P0` 全局协作约束：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
- `P1` 当前上下文：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- `P2` 能力文档（按需打开）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/`
- `P3` 归档区：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`

## 稳定工作流文档
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/agent-doc-governance.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-scaffold-regression-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`

## 定向架构文档
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.zh-CN.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md`

## 定向检索命令
```bash
# 最近重构文档
rg --files docs/refactoring | sort | tail -n 30

# 按能力关键词查文档
rg -n "permission|automation|app_scope|effects|wasm" docs/refactoring docs/automation docs/architecture

# 文档体积治理
./tools/docs/doc-hygiene-check.sh --strict
```

## AI 上下文路由器
```bash
# 重新生成机器索引 + 人类导航图
./tools/docs/ai-context.sh index

# 按任务生成最小读取集
./tools/docs/ai-context.sh route --task "automation gesture debug"

# 校验索引是否与 AGENTS/docs 同步（门禁用）
./tools/docs/ai-context.sh check --strict
# 可选：同时开启文档行数硬门禁
./tools/docs/ai-context.sh check --strict --enforce-line-limits

# 本地实时监听并自动刷新索引
./tools/docs/ai-context.sh watch
# 可选：安装 pre-commit 自动刷新与校验
./tools/docs/install-git-hook.sh
```
- 生成产物：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-index.json`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-map.md`
- 约束：
  - `route` 固定包含首读基线（P0 + P1），再按关键词和 token 预算补充 P2。
  - `check` 在 AGENTS/docs 已变化但未刷新索引时失败。

## macOS 本地命令
```bash
./mfx run
./mfx run-no-build
./mfx run-no-build --seconds 30
./mfx effects
./mfx verify-effects
./mfx verify-full
# 兼容旧命令
./mfx start
./mfx fast
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
./tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
./tools/platform/regression/run-theme-catalog-surface-regression.sh
```

## 维护规则
顶层索引只保留导航，不记录过程细节；细节放在对应能力文档。
