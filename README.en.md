# MFCMouseEffect

<p align="center">
  <img src="./MFCMouseEffect/res/logo_elegant.png" width="128" alt="MFCMouseEffect Logo">
</p>

<p align="center">
  <a href="https://github.com/sqmw/MFCMouseEffect/releases/latest"><img src="https://img.shields.io/badge/release-latest-blue" alt="release"></a>
  <img src="https://img.shields.io/badge/status-active%20development-green" alt="status">
  <img src="https://img.shields.io/badge/license-MIT-brightgreen" alt="license">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20core%20lane%20%7C%20Linux%20gate-lightgrey" alt="platform">
</p>

**[🇨🇳 中文](README.md)** | **🇬🇧 English**

---

`MFCMouseEffect` is a desktop input-visualization and interaction-feedback engine:
- Mouse effects (`click / trail / scroll / hold / hover`)
- Input indicator (mouse + keyboard labels)
- Automation mapping (mouse actions + drag gestures -> keyboard shortcut injection)
- WASM effect plugin runtime (load/reload/diagnostics/import/export)
- Shared Web settings UI (Svelte, cross-platform)

## Preview

| | |
| :---: | :---: |
| <img src="./docs/images/setting_en.png" width="340"><br>Settings page | <img src="./docs/images/ripple_concept.png" width="340"><br>Click ripple |
| <img src="./docs/images/trail_concept.png" width="340"><br>Trail effect | <img src="./docs/images/scroll_concept.png" width="340"><br>Scroll feedback |
| <img src="./docs/images/hold_concept.png" width="340"><br>Hold feedback | <img src="./docs/images/hover_concept.png" width="340"><br>Hover feedback |

## Platform Status

| Platform | Status | Notes |
| :--- | :--- | :--- |
| Windows 10+ | Stable mainline | Full capability set, regression compatibility preserved |
| macOS | Active mainline (core lane) | Effects + indicator + automation/gesture + WASM contracts are continuously hardened |
| Linux | Follow lane | Compile gate + contract coverage focused, not the primary full-experience lane |

> Current iteration priority is `macOS mainline first`, while keeping Windows behavior regression-free.

## Capability Overview

### 1) Effects System
- Five interaction channels: `click / trail / scroll / hold / hover`
- Cross-platform type normalization and config mapping (Win/mac parity)
- Ongoing macOS improvements (including `trail line` continuity and `click=text` fallback semantics)
- WebSettings exposes effect switching, tuning, and diagnostics

### 2) Input Indicator
- Visual feedback for mouse click, wheel, and keyboard labels
- Relative/absolute positioning, monitor targeting, and multi-screen overrides
- Test probes and regression hooks for observability

### 3) Automation + Gesture Recognition
- Mouse action mappings: left/right/middle click and wheel up/down -> shortcuts
- Gesture mappings: drag direction chains (for example `up_right`, `down_left_up`) -> shortcuts
- Configurable gesture trigger button, minimum stroke distance, sample step, and max direction segments
- App scopes (`all/selected`) and deterministic binding priority rules

### 4) WASM Effect Plugins
- Manifest load/reload, folder import, and bulk export flows
- Stable `error_code` model surfaced to WebUI
- Runtime diagnostics for budget/parse/render/load-failure (`stage/code`)
- Test-gated APIs and regression scripts for non-interactive verification

### 5) Shared WebSettings Modules
The current settings page is split into focused modules:
- `General`
- `Active Effects`
- `Input Indicator`
- `Text Content (Click/Text)`
- `Automation Mapping`
- `Effect Plugins (WASM)`
- `Trail Tuning`

## Quick Start

### Windows (Visual Studio)
1. Open `MFCMouseEffect.slnx` with Visual Studio 2026
2. Select `Release | x64` and rebuild
3. Run `x64/Release/MFCMouseEffect.exe`

### macOS (Daily Dev Shortcuts)
```bash
# Build + run core host (default auto-stop: 30 minutes)
./mfx start

# Run without rebuilding core/WebUI
./mfx fast

# Effects type parity selfcheck
./mfx effects
```

## Regression and Selfcheck Entries

```bash
# Full POSIX suite (scaffold + core + linux gate)
./tools/platform/regression/run-posix-regression-suite.sh --platform auto

# Effects-focused suite
./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto

# WASM-focused suite
./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto
```

```bash
# macOS WebSettings manual runner
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60

# macOS WASM runtime selfcheck
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build

# macOS automation injection selfcheck
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
```

## Repository Map

- `MFCMouseEffect/MouseFx`: core engine (effects, input, automation, WASM, WebSettings server)
- `MFCMouseEffect/Platform`: platform implementations (Windows/macOS/Linux)
- `MFCMouseEffect/WebUIWorkspace`: Svelte settings UI source
- `tools/platform/regression`: cross-platform regression scripts
- `tools/platform/manual`: macOS manual/selfcheck scripts
- `docs`: architecture, refactoring, issues, and regression docs

## Docs Entry

- Documentation index (EN): `./docs/README.md`
- Agent active context: `./docs/agent-context/current.md`
- Roadmap snapshot: `./docs/refactoring/phase-roadmap-macos-m1-status.md`

## License

[MIT License](./LICENSE)

---
<p align="center"><b>If it helps, please <a href="https://github.com/sqmw/MFCMouseEffect">star ⭐</a> and share feedback in Issues/Discussions.</b></p>
