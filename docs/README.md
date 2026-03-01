# MFCMouseEffect Documentation

Language: [English](README.md) | [中文](README.zh-CN.md)

## Why This Index
This file is intentionally compact for AI-IDE and human quick navigation.
Use it as a pointer index, not a full historical catalog.

## Read Order (Agent-First)
1. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
2. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
3. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
4. One targeted issue/refactoring doc for current task

## High-Priority Docs
- Active context: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
- Roadmap snapshot: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- Doc governance: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/agent-doc-governance.md`
- POSIX regression suite workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- POSIX scaffold workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-scaffold-regression-workflow.md`
- POSIX core smoke workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- POSIX core automation contract workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`
- POSIX Linux compile gate workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`

## Recent Issue Docs
- macOS trail “none” selection not sticking: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/macos-trail-none-selection-apply.md`
- macOS trail `streamer/line` regression fix: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/macos-trail-streamer-line-regression-fix.md`
- macOS line-trail Swift bridge cutover: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase56zb-macos-line-trail-swift-bridge-cutover.md`
- macOS effects Swift-bridge path dedup: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase56zc-macos-effects-swift-bridge-path-dedup.md`
- macOS window-registry ObjC++ detach: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase56zd-macos-window-registry-objcxx-detach.md`

## Current macOS Mainline (Phases 50-55)
- Dual-lane guardrails: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase50-posix-core-runtime-dual-lane-guardrails.md`
- Core decoupling: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase51-core-win32-decoupling-and-posix-path-foundation.md`
- Permission and input convergence (52x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52f-macos-runtime-permission-revocation-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52j-macos-startup-missing-permission-retry-and-notify-dedup.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52k-macos-permission-and-indicator-contract-automation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52l-macos-scroll-baseline-effect.md`
- Automation and app-scope (53x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53a-macos-automation-system-services-bootstrap.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53b-automation-appscope-cross-platform-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53d-macos-shortcut-capture-keycode-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53f-macos-automation-scope-ui-platform-semantics.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53i-macos-automation-injection-selfcheck-and-match-inject-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53ai-automation-mapping-phase-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53aj-macos-automation-app-scope-selfcheck-script.md`
- Linux follow gates (54x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54c-posix-regression-suite-orchestrator.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54f-core-automation-http-contract-regression.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54g-macos-automation-injection-selfcheck-suite-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54h-linux-dual-lane-compile-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54i-linux-follow-phase-closure.md`
- WASM runtime and diagnostics (55x):
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
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzal-macos-wasm-text-overlay-layout-style-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzae-posix-shell-exit-command-and-scaffold-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzaf-scaffold-settings-api-and-routecodec-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzao-macos-hover-overlay-core-plan-layers-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzza-macos-wasm-overlay-render-math-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzap-macos-trail-overlay-core-plan-layers-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzc-macos-keyboard-injector-keytables-responsibility-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzat-gesture-recognizer-direction-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzze-macos-global-input-hook-eventtap-dispatch-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzi-macos-hold-overlay-core-state-update-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzz-macos-input-indicator-show-plan-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzam-macos-click-overlay-core-plan-layers-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzan-macos-scroll-overlay-core-plan-layers-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzay-macos-effects-tempo-profile-parity.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzba-macos-effect-overlay-frame-clamp-multiscreen.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbb-macos-effect-overlay-contents-scale-consistency.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbc-macos-effect-animation-opacity-policy-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbd-macos-effect-geometry-metric-scaling.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbe-macos-effect-duration-size-strength-mapping.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbf-macos-effect-alpha-policy-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbg-objcxx-edit-policy-gate.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbh-posix-regression-suite-options-phase-split.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbi-posix-regression-cli-value-contract-hardening.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbj-macos-user-notification-swift-bridge.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbk-macos-folder-picker-swift-bridge.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbl-macos-settings-launcher-swift-bridge.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbm-macos-user-notification-native-center.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbn-macos-click-color-config-parity.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbo-macos-scroll-hover-color-profile-parity.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbp-macos-trail-hold-color-profile-parity.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbq-effects-profile-color-contract-expansion.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbr-macos-effects-type-tempo-variant-parity.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbs-effects-tempo-contract-surfacing.md` + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbt-effects-profile-cross-api-parity-check.md`
  - Full hardening sequence (`55h-55zzx`) is summarized in `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`

## Targeted Retrieval Commands
```bash
# List latest refactoring docs
rg --files docs/refactoring | sort | tail -n 30

# Search by capability keyword
rg -n "permission|automation|app_scope|wasm|throttle" docs/refactoring docs/issues

# Token hygiene check (weekly)
./tools/docs/doc-hygiene-check.sh --strict

# macOS one-command shortcuts (repo root)
./mfx start
./mfx fast
./mfx effects

# macOS core manual runner (URL/token auto-resolved)
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60

# macOS wasm runtime selfcheck (load/invoke/render/fallback)
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build

# macOS core wasm-focused HTTP contract gate (faster than full automation contract run)
./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto

# macOS automation app-scope alias selfcheck (`code.exe/code/code.app`)
./tools/platform/manual/run-macos-automation-app-scope-selfcheck.sh --skip-build

# macOS automation injection selfcheck (real dispatch by default; add --dry-run for deterministic mode)
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build

# macOS effects type parity selfcheck (5 categories + alias normalization)
./tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build

# macOS ObjC++ surface regression gate (.mm zero + explicit allowlist compile policy)
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

## Archive
- Archive root: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`
- For complete historical coverage, use targeted `rg` queries instead of expanding this top-level index.
