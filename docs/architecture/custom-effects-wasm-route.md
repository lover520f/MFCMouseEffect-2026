# Custom Effects (WASM Route) Architecture

## Goal

Enable high-flexibility user-defined effects without giving up native performance:
- User logic runs in WASM plugins.
- Rendering and resource ownership stay in the C++ host.

This route targets:
- custom click/text/image behavior,
- deterministic frame pacing,
- user-local plugin compilation (`.wasm` artifacts).

## Non-goals (current stage)

- No “arbitrary JS directly to WASM at runtime”.
- No direct plugin control over swapchain/window composition.
- No visual node editor in v1.

## Architecture

1. `Event Capture (C++)`
- Normalize click/wheel/hold/gesture input.

2. `Wasm Effect Host (C++)`
- Load wasm module, call stable C ABI exports.
- Collect command buffer output.
- Enforce runtime budgets.

3. `Render Execution (C++)`
- Convert commands to host-native effect objects.
- Execute through existing batched render path.

Principle:
- WASM computes logic; C++ renders.
- Cross-boundary calls are event-batched, not object-granular.

## Plugin delivery model

User workflow:
1. Write plugin with provided template (AssemblyScript-style recommended).
2. Compile locally to `effect.wasm`.
3. Provide `plugin.json` manifest.
4. Enable plugin in settings.

Current official template location:
- `examples/wasm-plugin-template`
- quick start: `docs/architecture/wasm-plugin-template-quickstart.md`
- compatibility policy: `docs/architecture/wasm-plugin-compatibility.md`
- troubleshooting: `docs/architecture/wasm-plugin-troubleshooting.md`

Example manifest:

```json
{
  "id": "demo.click.emojis",
  "name": "Demo Emoji Click",
  "version": "0.1.0",
  "api_version": 1,
  "entry": "effect.wasm"
}
```

## ABI v1 draft

```c
uint32_t mfx_plugin_get_api_version(void);

// Recommended unified event entry:
// kind = click/move/scroll/hold-start/hold-update/hold-end/hover-start/hover-end
uint32_t mfx_plugin_on_event(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);

void mfx_plugin_reset(void);
```

Compatibility rule:
- plugin exports must include `mfx_plugin_get_api_version` and `mfx_plugin_on_event`.

## Command buffer v1 draft

MVP command types:
- `spawn_text`
- `spawn_image`

Shared fields:
- transform: `x,y,scale,rotation`
- motion: `vx,vy,ax,ay`
- style: `alpha,color`
- life: `delay_ms,life_ms`
- resource: `text_id` or `image_id`

## Performance budgets (initial)

- Plugin CPU per event: `<= 1.0 ms`
- Commands per event: `<= 256`
- Plugin linear memory: `<= 4 MB`
- New objects per frame: `<= 512`

Fallback rules:
- timeout => drop this event output
- command overflow => truncate
- repeated failure => disable plugin and fallback to built-in effect
- runtime bridge fallback visibility: inspect `wasm.runtime_backend` and `wasm.runtime_fallback_reason`

## Packaging impact

Recommended runtime class: lightweight wasm runtime (WAMR/wasm3 class).

Expected installer delta target:
- about `1-3 MB` (runtime + host bridge + docs/templates)

Toolchain is user-local, not bundled in installer.

## Delivery phases

### Phase 1
- Add `WasmEffectHost` skeleton.
- Finalize ABI v1 and I/O structures.
- Click path calls plugin and logs decoded commands (no rendering hookup yet).

### Phase 2
- Hook command execution into existing click text/image path.
- Add budget enforcement and diagnostics.

### Phase 3
- Web settings: plugin select/enable/reload/diagnostics.
- Plugin directory scan and manifest validation.

### Phase 4
- Publish official template and compile guide.
- Versioned compatibility docs.

## Decision

Adopt:
- `WASM logic plugins + C++ host rendering`.

Do not use as mainline:
- WebView render chain.
- Native DLL plugin route for general users.

## Delivery progress (15 commits)

Implementation is split into small commits: architecture skeleton first, then event-chain and Web integration.

- [x] Commit 1: `MouseFx/Core/Wasm` skeleton (Host/Runtime/ABI) and project wiring
- [x] Commit 2: ABI v1 input/output serialization and command-buffer parser
- [x] Commit 3: Plugin manifest model (`plugin.json`) and validator
- [x] Commit 4: Plugin discovery and path strategy (default + config directory)
- [x] Commit 5: WasmEffectHost lifecycle hardening (load/unload/reload)
- [x] Commit 6: `AppController` startup/shutdown integration (init only, no behavior change)
- [x] Commit 7: Click event-chain invocation (logging only, no render path yet)
- [x] Commit 8: Budget controls (latency/output size/command count) and diagnostics
- [x] Commit 9: Diagnostics mapping into settings state output
- [x] Commit 10: Read-only WASM state exposure in Web API
- [x] Commit 11: Plugin enable/disable/reload command endpoints (IPC/HTTP)
- [x] Commit 12: RuntimeFactory extension to a real WASM runtime (keep Null fallback)
- [x] Commit 13: Official template + local compile script examples
- [x] Commit 14: Docs hardening (quick start/troubleshooting/compatibility)
- [x] Commit 15: Regression and stabilization pass (default-off + fallback validation)

## True Delivery Progress (5 commits)

Note: the prior 15 commits established architecture baseline; these 5 commits wire visible runtime rendering.

- [x] Commit A1: execute `spawn_text` in click path (WASM text takes precedence when commands are emitted)
- [x] Commit A2: execute `spawn_image` in click path (image/icon command rendering)
- [x] Commit A3: resource mapping for `text_id/image_id` with deterministic fallback
- [x] Commit A4: harden render-execution with budget/error fallback coupling
- [x] Commit A5: end-to-end validation and documentation closure

## Incremental Follow-up (Phase 3 UI)

- [x] Phase 3a: Web settings WASM panel wired
  - Added `/api/wasm/catalog`
  - Added Svelte section for plugin catalog + enable/disable/reload/load-manifest
  - Added i18n, build, and installer preflight wiring for `wasm-settings.svelte.js`
- [x] Phase 3b: policy controls in settings (fallback toggle/profile-level behavior)
  - Added persisted `wasm` config (`enabled`, `fallback_to_builtin_click`, `manifest_path`)
  - Added `/api/wasm/policy` and command `wasm_set_policy`
  - Startup now restores manifest path and enabled state
  - Click dispatch now honors fallback policy when WASM route is active
  - Details: `docs/issues/wasm-web-settings-policy-phase3b.md`
- [x] Phase 3c: configurable WASM execution budget policy
  - Added persisted budget policy (`output_buffer_bytes`, `max_commands`, `max_execution_ms`)
  - Runtime host budget now synced from config on startup/reload/policy update
  - Web panel supports budget editing and runtime snapshot display
  - Details: `docs/issues/wasm-web-settings-budget-policy-phase3c.md`
- [x] Phase 3d: schema-driven budget inputs and default restore
  - Removed hard-coded budget bounds in Web panel (`min/max/step` from schema)
  - Added shared policy model for range normalization and clamp/snap
  - Added `Reset Defaults` policy action and server-side default-range alignment
  - Details: `docs/issues/wasm-web-settings-budget-schema-phase3d.md`
- [x] Phase 3e: runtime diagnostics visualization
  - Exposed budget telemetry fields in WASM panel (call metrics/flags/reason/parse error)
  - Added diagnostics model extraction for normalization + warning classification
  - Added warning highlight style and localized labels
  - Details: `docs/issues/wasm-web-settings-diagnostics-phase3e.md`
- [x] Phase 3f: state-first refresh after WASM actions
  - Replaced per-action full reload with state-first refresh path for WASM operations
  - Reuses cached schema when language is unchanged, reducing redundant schema requests
  - Keeps full reload fallback for robustness when local refresh fails
  - Details: `docs/issues/wasm-web-settings-state-refresh-phase3f.md`

## Runtime Bridge Build Status

- [x] `mfx_wasm_runtime.dll` is now built from this repo (no external download step).
- Build integration:
  - `MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj`
  - `MFCMouseEffect.slnx` + `MFCMouseEffect/MFCMouseEffect.vcxproj` project reference
- Installer integration:
  - `Install/MFCMouseEffect.iss` preflight + file packaging for `mfx_wasm_runtime.dll`
- Details: `docs/issues/wasm-runtime-bridge-self-build.md`

## Incremental Follow-up (Phase 4 Template Ecosystem)

- [x] Phase 4a: official sample-preset matrix and build script split
  - Added shared AssemblyScript modules (`common/abi.ts`, `common/random.ts`)
  - Added preset samples (`text-rise`, `text-burst`, `image-pulse`, `mixed-text-image`)
  - Added script toolchain (`build-lib`, `build-sample`, `build-all-samples`)
  - Added pnpm-friendly compiler path probing (`asc.js` and legacy `asc`)
  - Details: `docs/issues/wasm-plugin-template-sample-presets-phase4.md`
- [x] Phase 4b: user-facing structure docs and expanded sample coverage
  - Expanded presets to a fuller matrix (`text-spiral`, `text-wave-chain`, `image-burst`, `image-lift`, `mixed-emoji-celebrate`, `button-adaptive`)
  - Added sample-level `image_assets` manifest emission in sample build scripts
  - Added bilingual template docs (`examples/wasm-plugin-template/README.md`, `examples/wasm-plugin-template/README.zh-CN.md`)
  - Details: `docs/issues/wasm-plugin-template-full-sample-matrix-phase4b.md`
- [x] Phase 4c: real image asset pack and format-complete sample outputs
  - Added template asset pack with all supported formats (`png/jpg/jpeg/bmp/gif/tif/tiff`)
  - Sample build scripts now copy declared `image_assets` into output folders
  - Preset mappings updated to exercise all formats in sample manifests
  - Details: `docs/issues/wasm-plugin-template-assets-all-formats-phase4c.md`
