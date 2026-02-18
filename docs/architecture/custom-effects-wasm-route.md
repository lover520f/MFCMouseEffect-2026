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

uint32_t mfx_plugin_on_click(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);

void mfx_plugin_reset(void);
```

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
- [ ] Commit 8: Budget controls (latency/output size/command count) and diagnostics
- [ ] Commit 9: Diagnostics mapping into settings state output
- [ ] Commit 10: Read-only WASM state exposure in Web API
- [ ] Commit 11: Plugin enable/disable/reload command endpoints (IPC/HTTP)
- [ ] Commit 12: RuntimeFactory extension to a real WASM runtime (keep Null fallback)
- [ ] Commit 13: Official template + local compile script examples
- [ ] Commit 14: Docs hardening (quick start/troubleshooting/compatibility)
- [ ] Commit 15: Regression and stabilization pass (default-off + fallback validation)
