# MFCMouseEffect 文档

语言： [English](README.md) | [中文](README.zh-CN.md)

## 索引目标
本文件保持精简，优先服务 AI-IDE 与人工快速定位。
它是“导航入口”，不是“全量历史清单”。

## 建议阅读顺序（Agent-First）
1. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
2. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
3. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
4. 当前任务对应的一篇 issue/refactoring 文档

## 高优先文档
- 当前上下文：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
- 路线图快照：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- 文档治理：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/agent-doc-governance.md`
- POSIX 回归总控：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- POSIX scaffold 回归：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-scaffold-regression-workflow.md`
- POSIX core smoke：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- POSIX core automation 合同：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`
- POSIX Linux 编译门禁：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`

## 当前 macOS 主线（Phase 50-55）
- 双车道护栏：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase50-posix-core-runtime-dual-lane-guardrails.md`
- core 去 Win32 强耦合：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase51-core-win32-decoupling-and-posix-path-foundation.md`
- 权限与输入收敛（52x）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52f-macos-runtime-permission-revocation-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52j-macos-startup-missing-permission-retry-and-notify-dedup.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52k-macos-permission-and-indicator-contract-automation.md`
- 自动化与 app_scope（53x）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53a-macos-automation-system-services-bootstrap.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53e-macos-automation-app-catalog-scan.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53c-posix-core-settings-webui-rewire.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53d-macos-shortcut-capture-keycode-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53b-automation-appscope-cross-platform-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53f-macos-automation-scope-ui-platform-semantics.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53g-appscope-match-test-endpoint-and-contract-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53h-automation-binding-priority-contract-closure.md`
- Linux 跟随门禁（54x）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54c-posix-regression-suite-orchestrator.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54f-core-automation-http-contract-regression.md`
- WASM 运行时与诊断（55x）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55h-macos-wasm-overlay-throttle-guardrail.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55j-macos-wasm-throttle-cause-breakdown.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55k-macos-wasm-async-task-lifetime-crash-fix.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55l-posix-wasm-catalog-transfer-apis.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55m-macos-native-folder-picker-wasm-import-dialog.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55n-wasm-dispatch-test-contract-automation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-macos-manual-websettings-runner.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-b-macos-wasm-runtime-selfcheck-script.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-c-macos-folder-picker-implementation-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-macos-wasm-closure.md`

## 定向检索命令
```bash
# 查看最新重构文档
rg --files docs/refactoring | sort | tail -n 30

# 按能力关键词检索
rg -n "permission|automation|app_scope|wasm|throttle" docs/refactoring docs/issues

# 文档体积治理（建议每周执行）
./tools/docs/doc-hygiene-check.sh --strict

# macOS core 手测一键入口（自动解析 URL/token）
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60

# macOS wasm 运行时自检（load/invoke/render/fallback）
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build
```

## 归档区
- 归档入口：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`
- 全量历史请用 `rg` 定向检索，不再扩展顶层索引体积。
