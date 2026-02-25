# Phase 53r - SettingsStateMapper Sections Split

## Background
- `SettingsStateMapper.cpp` mixed multiple responsibilities:
  - base config state serialization
  - GPU route diagnostics state
  - WASM diagnostics state
  - input-capture diagnostics state
  - top-level response composition and apply bridge
- This created high coupling for state-schema evolution.

## Decision
- Keep JSON schema and behavior unchanged.
- Split mapper internals into dedicated section modules:
  - `SettingsStateMapper.BaseSections.*`
  - `SettingsStateMapper.Diagnostics.*`
- Keep `SettingsStateMapper.cpp` as thin composition + apply bridge.

## Code Changes
1. Base state sections module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.BaseSections.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.BaseSections.cpp`
- Extracted config serialization sections:
  - ui/theme/active
  - text content/font
  - trail style/profiles/params
  - input indicator
  - automation mappings/gesture mappings

2. Diagnostics sections module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.Diagnostics.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.Diagnostics.cpp`
- Extracted runtime/diagnostics sections:
  - GPU route status snapshot + notice
  - WASM runtime diagnostics state
  - input-capture runtime state + notice

3. Main mapper simplified
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.cpp`
- Now composes:
  - base sections
  - diagnostics sections
  - `input_capture_notice` mirror field
- `ApplySettingsStateJson(...)` kept intact as command bridge.

4. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added new split source files into runtime source list.

## Behavior Compatibility
- Output field names, structure, and semantics remain unchanged.
- This is structure-only refactor, no contract changes.

## Functional Ownership
- Category: `共用控制面`
- Coverage: state mapping used by `特效 / WASM / 键鼠指示 / 手势映射`

## Verification
1. `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- Result: passed.

2. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.
