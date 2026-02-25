# Phase 53u - WebSettings Test Automation Routes Layer Split

## Background
- `WebSettingsServer.TestAutomationApiRoutes.cpp` had grown to include three concerns in one file:
  - app-scope and binding-priority matching probes
  - matcher + injection probe routes
  - mac keycode shortcut mapping probe route
- This made test-route evolution high-coupling and increased review cost.

## Decision
- Keep all test API paths and payload contracts unchanged.
- Split automation test routes by responsibility:
  - `scope` routes
  - `injection` routes
  - `shortcut` routes
- Extract shared parsing/normalization helpers into route utils.
- Keep `WebSettingsServer.TestAutomationApiRoutes.cpp` as thin delegating entry.

## Code Changes
1. Added scoped route module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationScopeApiRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationScopeApiRoutes.cpp`
- Owns:
  - `POST /api/automation/test-app-scope-match`
  - `POST /api/automation/test-binding-priority`

2. Added injection route module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationInjectionApiRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationInjectionApiRoutes.cpp`
- Owns:
  - `POST /api/automation/test-match-and-inject`
  - `POST /api/automation/test-inject-shortcut`

3. Added shortcut route module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationShortcutApiRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationShortcutApiRoutes.cpp`
- Owns:
  - `POST /api/automation/test-shortcut-from-mac-keycode`

4. Added shared helper module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationRouteUtils.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationRouteUtils.cpp`
- Shared:
  - env switch checks
  - payload parsers (`app_scopes`, `history`, `mappings`, `keys`)
  - normalized history builder
  - selected-binding JSON builder

5. Delegator and build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestAutomationApiRoutes.cpp`
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Behavior Compatibility
- Test route URLs, request schema, and response fields are unchanged.
- Feature flags (`MFX_ENABLE_AUTOMATION_SCOPE_TEST_API`, `MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API`, `MFX_ENABLE_AUTOMATION_INJECTION_TEST_API`) are unchanged.
- This phase is structure-only refactor.

## Functional Ownership
- Category: `手势映射`
- Coverage: automation matcher/injector test contracts and shortcut mapping probe contracts.

## Verification
1. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.
