# Custom Effects (WASM Route)

## Purpose
Define the stable architecture contract for custom effects:
- Plugin logic in WASM.
- Rendering/resource ownership in host C++.
- Behavior parity and runtime safety before visual polish.

This document is intentionally compact. Historical per-phase details are kept in targeted issue docs.

## Scope
- In scope:
  - click/text/image custom logic,
  - host-side budget enforcement,
  - Web settings policy/diagnostics,
  - plugin template and local build workflow.
- Out of scope:
  - direct JS-to-WASM runtime translation,
  - plugin control of host window/swapchain,
  - visual node editor.

## Data Flow
1. Host captures normalized input events.
2. `WasmEffectHost` calls plugin ABI (`on_event`) with event payload.
3. Plugin returns command buffer.
4. Host validates budget and command schema.
5. Host renderer executes supported commands and applies fallback on failure.

Core rule: WASM computes; C++ executes.

## Plugin Contract (ABI v1)
Required exports:

```c
uint32_t mfx_plugin_get_api_version(void);
uint32_t mfx_plugin_on_event(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);
void mfx_plugin_reset(void); // optional but recommended
```

Event kinds follow normalized host semantics (`click/move/scroll/hold*/hover*`).

## Command Contract (v1)
Supported render commands (current production path):
- `spawn_text`
- `spawn_image`

Common fields:
- transform: `x, y, scale, rotation`
- motion: `vx, vy, ax, ay`
- style: `alpha, color`
- lifecycle: `delay_ms, life_ms`
- resource selector: `text_id` or `image_id`

## Runtime Budgets and Fallback
Default budgets:
- `max_execution_ms <= 1.0`
- `max_commands <= 256`
- `output_buffer_bytes` policy-bound

Fallback policy:
- timeout: drop current event output
- overflow: truncate command list
- repeated failure: disable plugin route and fallback to built-in effect

Observable runtime state:
- `wasm.runtime_backend`
- `wasm.runtime_fallback_reason`

## Current Delivery State
- Runtime route, diagnostics, and fallback are active in settings/state.
- Policy controls (`enabled`, `manifest_path`, `fallback_to_builtin_click`, budget fields) are persisted.
- Template ecosystem (presets/assets/build scripts) is published under:
  - `examples/wasm-plugin-template`

## Source of Truth Docs
- Template quickstart:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-template-quickstart.md`
- Compatibility:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-compatibility.md`
- Troubleshooting:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-troubleshooting.md`
- Incremental phase notes:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-policy-phase3b.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-budget-policy-phase3c.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-budget-schema-phase3d.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-diagnostics-phase3e.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-web-settings-state-refresh-phase3f.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-runtime-bridge-self-build.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-plugin-template-sample-presets-phase4.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-plugin-template-full-sample-matrix-phase4b.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/wasm-plugin-template-assets-all-formats-phase4c.md`
