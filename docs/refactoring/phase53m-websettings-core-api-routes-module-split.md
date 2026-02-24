# Phase 53m - WebSettings Core API Routes Module Split

## Background
- After phase53l, `WebSettingsServer.Routing.cpp` still handled core settings APIs directly.
- Main routing file still mixed top-level dispatch concerns with concrete core API behavior.

## Decision
- Keep endpoint behavior unchanged.
- Move core settings APIs into a dedicated module:
  - `WebSettingsServer.CoreApiRoutes.h`
  - `WebSettingsServer.CoreApiRoutes.cpp`
- Keep `stop` action decoupled from `WebSettingsServer` by injecting a callback:
  - `const std::function<void()>& stopAsync`

## Code Changes
1. New core API route module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.CoreApiRoutes.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.CoreApiRoutes.cpp`
- Moved endpoint handlers:
  - `GET /api/schema`
  - `GET /api/state`
  - `POST|GET /api/reload`
  - `POST /api/stop`
  - `POST /api/reset`
  - `POST /api/state`

2. Main route file simplification
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- Removed inline core-route branches.
- Added delegation call:
  - `HandleWebSettingsCoreApiRoute(req, path, controller_, [this]() { StopAsync(); }, resp)`

3. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added `WebSettingsServer.CoreApiRoutes.cpp` to runtime source list.

## Behavior Compatibility
- Core API path contracts and response payloads remain unchanged.
- Stop behavior remains async; implementation moved behind callback injection only.
- This phase is a structural refactor, not a behavior change.

## Verification
1. `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- Result: passed.

2. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.

## Risks
- Risk: callback wiring could accidentally change stop behavior.
- Mitigation: full POSIX suite includes scaffold/core smoke and core contract checks, covering startup/exit and API paths after refactor.
