# Phase 53s - SettingsSchemaBuilder Sections Split

## Background
- `SettingsSchemaBuilder.cpp` still combined option-list building and capability/wasm schema building in one file.
- This increased coupling when expanding schema for different capability domains.

## Decision
- Keep schema output contracts unchanged.
- Split schema construction into section modules:
  - `OptionsSections` for selectable options and monitor/effect catalogs
  - `CapabilitiesSections` for wasm policy/diagnostics keys and platform capability flags
- Keep `SettingsSchemaBuilder.cpp` as thin composition entry.

## Code Changes
1. Option sections module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.OptionsSections.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.OptionsSections.cpp`
- Extracted:
  - UI language/theme/follow-mode/presenter backend options
  - input-indicator and automation option lists
  - monitor enumeration payload
  - effect metadata option arrays

2. Capabilities sections module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.CapabilitiesSections.h`
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.CapabilitiesSections.cpp`
- Extracted:
  - wasm policy/diagnostic schema section
  - platform capabilities section

3. Main builder simplification
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.cpp`
- Now only composes option and capability sections, then serializes JSON.

4. Build wiring
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added new section source files to runtime source list.

## Behavior Compatibility
- Schema key names and value semantics unchanged.
- This phase is structure-only refactor.

## Functional Ownership
- Category: `共用控制面`
- Coverage: schema consumed by `特效 / WASM / 键鼠指示 / 手势映射`

## Verification
1. `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- Result: passed.

2. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.
