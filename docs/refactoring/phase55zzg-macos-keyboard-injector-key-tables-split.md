# Phase 55zzg: macOS Keyboard Injector Key Tables Split

## Capability
- 手势映射（Automation）

## Why
- `MacosKeyboardInjectorKeyResolver.mm` simultaneously owned:
  - key mapping tables (printable/function/special/modifier)
  - resolver orchestration (priority, output assembly)
- This mixed data mapping and flow control responsibilities, making keyboard mapping evolution harder to review and reuse.

## Scope
- Keep keyboard injection behavior unchanged.
- Extract all vk->mac mapping tables into dedicated module.
- Keep resolver file focused on parsing/orchestration flow.

## Code Changes

### 1) New key-table module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosKeyboardInjectorKeyTables.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosKeyboardInjectorKeyTables.mm`
- Owns:
  - printable key mapping (`A-Z`, `0-9`, numpad)
  - function key mapping (`F1-F20`)
  - special key mapping (tab/enter/arrow/page/home/end/delete...)
  - modifier mapping (`Shift/Ctrl/Alt/Cmd`)

### 2) Resolver simplification
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosKeyboardInjectorKeyResolver.mm`
- Keeps:
  - resolver API surface (`ResolveModifierMapping`, `ResolveKeyCode`)
  - mapping resolution priority and output assembly
  - null/argument validation

### 3) Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No behavior/schema/API change.
- Existing shortcut injection semantics (`Cmd`/modifier priority/order) remain unchanged.
