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
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52l-macos-scroll-baseline-effect.md`
- 自动化与 app_scope（53x）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53a-macos-automation-system-services-bootstrap.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53b-automation-appscope-cross-platform-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53d-macos-shortcut-capture-keycode-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53f-macos-automation-scope-ui-platform-semantics.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53i-macos-automation-injection-selfcheck-and-match-inject-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53ai-automation-mapping-phase-closure.md`
  - 完整模块拆分链（`53j-53ah`）见：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- Linux 跟随门禁（54x）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54c-posix-regression-suite-orchestrator.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54f-core-automation-http-contract-regression.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54g-macos-automation-injection-selfcheck-suite-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54h-linux-dual-lane-compile-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54i-linux-follow-phase-closure.md`
- WASM 运行时与诊断（55x）：
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-macos-wasm-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55p-macos-wasm-selfcheck-suite-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55q-posix-wasm-load-failure-diagnostics-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zd-wasm-transfer-error-code-regression-matrix-and-i18n.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55ze-webui-wasm-error-model-test-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zf-wasm-focused-contract-gate-and-selfcheck-expansion.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zo-posix-platform-arg-helper-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zp-doc-index-compaction-p0-p1.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zq-core-regression-workflow-helper-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zr-wasm-dispatch-readiness-retry-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzs-wasm-manifest-path-trim-contract-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzt-wasm-selfcheck-helper-modularization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzu-core-http-wasm-helper-modularization-and-lock-race-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzv-core-http-wasm-contract-check-modularization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzw-core-http-automation-contract-check-modularization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzx-core-http-orchestrator-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzy-core-http-input-contract-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzz-http-entry-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzza-core-smoke-entry-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzb-macos-effects-category-parity-baseline.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzc-macos-effects-runtime-diagnostics-and-probe-expansion.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzd-macos-effect-type-probe-and-hold-style-mapping.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzze-settings-state-effects-diagnostics-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzf-websettings-test-effects-route-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzg-macos-hold-overlay-style-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzh-macos-trail-overlay-style-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzi-macos-click-overlay-style-expansion.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzj-macos-scroll-overlay-style-normalization-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzk-macos-hover-overlay-style-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzl-effects-profile-state-builder-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzm-effects-profile-and-diagnostics-multi-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzn-click-scroll-renderer-core-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzo-hold-trail-renderer-core-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzp-macos-hover-renderer-core-and-effect-registry-table-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzq-command-handler-apply-settings-responsibility-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzr-macos-wasm-image-overlay-renderer-core-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzs-macos-wasm-overlay-state-internals-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzt-wasm-plugin-transfer-service-responsibility-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzu-wasm-plugin-manifest-load-validate-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzv-wasm3-runtime-responsibility-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzw-wasm-effect-host-invoke-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzx-dispatch-router-pointer-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzy-appcontroller-vm-suppression-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzz-macos-system-scan-and-picker-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzza-macos-wasm-overlay-render-math-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzb-macos-wasm-command-render-dispatch-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzc-macos-keyboard-injector-keytables-responsibility-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzd-macos-applescript-folder-picker-script-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzze-macos-global-input-hook-eventtap-dispatch-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzf-macos-input-indicator-overlay-style-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzg-macos-wasm-image-overlay-renderer-support-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzh-macos-app-catalog-scan-workflow-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzi-macos-hold-overlay-core-state-update-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzj-macos-scroll-overlay-renderer-support-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzab-macos-wasm-overlay-admission-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzl-macos-overlay-coord-conversion-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzz-macos-input-indicator-show-plan-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzy-macos-user-notification-applescript-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzaa-macos-keyboard-injector-eventpost-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzs-macos-tray-menu-factory-action-items-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzt-macos-hold-style-accent-helper-split.md`
  - 完整 hardening 流水（`55h-55zzx`）见：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`

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

# macOS core wasm 聚焦 HTTP 合同门禁（比全量 automation 合同更快）
./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto

# macOS core wasm 路径裁剪合同门禁（快速验证路由路径语义）
./tools/platform/regression/run-posix-core-wasm-path-contract-regression.sh --platform auto

# macOS 自动化注入自检（默认真实注入；加 --dry-run 为确定性测试模式）
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
```

## 归档区
- 归档入口：`/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`
- 全量历史请用 `rg` 定向检索，不再扩展顶层索引体积。
