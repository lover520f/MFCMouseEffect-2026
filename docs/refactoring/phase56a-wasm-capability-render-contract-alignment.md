# phase56a: wasm render capability contract alignment

## Why
- `state.wasm.render_supported` already uses runtime renderer strategy (`SupportsRendering()`).
- `schema.capabilities.wasm.render` was still compile-time gated as windows-only.
- This made macOS settings/runtime diagnostics inconsistent.

## Change
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.CapabilitiesSections.cpp`.
- `capabilities.wasm.render` now uses runtime renderer capability:
  - create platform renderer once;
  - return `renderer && renderer->SupportsRendering()`.

## Impact
- macOS now reports wasm render capability consistently across schema and runtime diagnostics.
- Windows behavior is unchanged (`SupportsRendering()` remains true on windows renderer).
- linux/other platforms keep degraded renderer result (`false`) without hardcoded platform branching.

## Validation
- Build:
  - `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- Result:
  - build passed.
