# Phase 55zzi: macOS Effect Overlay Lifecycle Observability

## Capability
- 特效（Effect）

## Why
- Click/scroll overlay windows had runtime lifecycle logic, but lacked first-class diagnostics and script-level lifecycle checks.
- When overlay cleanup regresses, symptoms are visible in UI but hard to assert from contracts.

## Scope
- Keep effect rendering behavior unchanged.
- Add active overlay-window counters for click/scroll effect runtimes.
- Expose counters in `/api/state` and add test-only lifecycle probe API + regression assertions.

## Code Changes

### 1) Overlay registry runtime counters
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseWindowRegistry.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseWindowRegistry.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseWindowRegistry.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseWindowRegistry.mm`
- Added APIs:
  - `GetActiveClickPulseWindowCount()`
  - `GetActiveScrollPulseWindowCount()`

### 2) State/schema diagnostics exposure
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.Diagnostics.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.Diagnostics.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.CapabilitiesSections.cpp`
- Added `/api/state.effects_runtime` diagnostics:
  - `click_active_overlay_windows`
  - `scroll_active_overlay_windows`
  - `active_overlay_windows_total`
  - `total_matches_components`

### 3) Test-only effect lifecycle probe route
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsApiRoutes.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsApiRoutes.cpp`
- Updated route composition/build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestApiRoutes.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- New test endpoint:
  - `POST /api/effects/test-overlay-windows`
  - Supports optional emit (`click`/`scroll`) and wait-until-clear contract checks.

### 4) Regression contract hardening
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_checks.sh`
- Added checks:
  - `/api/state` must expose `effects_runtime` diagnostics fields.
  - Effect overlay probe must restore active-window count to baseline after emit+wait.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No behavior changes for end users.
- Runtime observability and regression coverage are strengthened for effect overlay lifecycle cleanup.
