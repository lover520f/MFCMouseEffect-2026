# WASM Demo Click Mixed Text Image (0.1.1) Execute Access Violation Fix

## Classification
- Result: `Bug / regression`
- Basis: process crash with execute access violation (`0xC0000005`) in WASM bridge execution path, not expected behavior.

## Symptom
- Running `Demo Click Mixed Text Image (0.1.1)` could terminate the process with:
  - `0xC0000005` execute access violation
  - fault address looked like a random executable pointer.

## Evidence
- Crash dump analysis pointed into `mfx_wasm_runtime.dll` runtime call path.
- Symbolized return addresses showed:
  - `op_MemGrow` in wasm3 execution
  - caller path from `RuntimeBridgeContext::CallOnClick`
- This matched complex click plugins that can trigger allocator/runtime memory growth.

## Root Cause
1. Variadic invocation path was used (`m3_CallV` / `m3_GetResultsV`) for host-to-wasm calls.
- In complex plugin paths this increases ABI/stack corruption risk and makes failures non-deterministic.

2. Host copied output using a wasm memory pointer captured before `on_click`.
- If plugin execution triggered `memory.grow`, previous linear-memory pointer could become stale.

3. No SEH guard around native runtime call boundaries.
- A low-level fault in runtime execution could directly crash the process before controlled error handling.

## Fix
1. Replaced variadic calls with typed APIs:
- `m3_Call`
- `m3_GetResults`

2. Hardened output memory handling:
- reacquire linear memory pointer after `on_click`
- revalidate output range against post-call memory size
- copy output only after bounds check.

3. Added protected runtime invoke wrappers:
- `SafeM3Call`
- `SafeM3GetResults`
- catch SEH at bridge boundary, convert to runtime error text.

4. Added runtime recovery path after protected fault:
- rebuild runtime from cached wasm bytes
- re-resolve plugin exports
- keep bridge in recoverable state for later calls.

5. Increased runtime stack budget:
- from `64 KB` to `256 KB` for complex plugin execution depth.

## Files
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.h`
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`

## Validation
1. Build validation (Release x64) passed:
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/WasmRuntimeBridge/mfx_wasm_runtime.vcxproj /p:Configuration=Release /p:Platform=x64 /m:1`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /p:Configuration=Release /p:Platform=x64 /m:1`

2. Runtime stress validation:
- `text-rise` sample: repeated `on_click` calls completed without process crash.
- `mixed-text-image` sample now returns deterministic runtime error (`missing imported function`) instead of hard crash when import preconditions are not met.

## Notes
- This change targets crash resilience and runtime safety boundaries.
- Any remaining plugin-side import mismatch should be handled as a separate compatibility task, not as a bridge crash.

## 2026-02-21 Follow-up: Mixed sample still reported as "exception"

### Updated diagnosis
- Result: still `Bug`, not an emoji/image-asset issue.
- Direct cause: unresolved WASM import `env.abort` in AssemblyScript-generated modules.

### Evidence
- `mixed-text-image/effect.wat` contains:
  - `(import "env" "abort" (func ... (param i32 i32 i32 i32)))`
- Before fix, runtime returned `missing imported function` on first `on_click`.

### Follow-up fix
- Added host import linking for:
  - `env.abort` signature `v(iiii)` (AssemblyScript abort ABI)
  - `env._abort` signature `v()` (compat fallback)
- Link stage is now executed in module load path before export resolution.

### Follow-up validation
- `mixed-text-image`: `1000` `on_click` calls succeeded.
- `image-affine-showcase`: `200` `on_click` calls succeeded.
- `text-rise`: `3000` `on_click` calls succeeded.

### Files (follow-up)
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.h`
- `MFCMouseEffect/WasmRuntimeBridge/WasmRuntimeBridgeContext.cpp`

## 2026-02-21 Compatibility follow-up: plugin command kind 3 not rendered

### Updated diagnosis
- Result: still `Bug`, not emoji/image-file corruption.
- Direct cause: host command parser only accepted command kinds `1`/`2`.
- New sample bundles (`image-affine-showcase`, `mixed-text-image`) emit image command kind `3` with `size=88`.

### Evidence
- Direct runtime call (`mfx_wasm_runtime_call_on_click`) returned valid payload:
  - `image-affine-showcase`: first command `kind=3,size=88`
  - `mixed-text-image`: command stream includes `kind=1,size=56` then `kind=3,size=88`
- Host parser treated `kind=3` as `UnsupportedCommandKind`, so render path returned no visible effect.

### Compatibility fix
- Added ABI enum support:
  - `CommandKind::SpawnImageAffine = 3`
- Added layout:
  - `SpawnImageAffineCommandV1` (`88` bytes, `SpawnImageCommandV1` prefix + affine tail fields)
- Parser now accepts kind `3` with minimum size `88`.
- Executor now handles kind `3` by reusing existing image render route with base image fields.
  - affine tail fields are preserved for future renderer extension (current host ignores them safely).

### Files (compat follow-up)
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmCommandBufferParser.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`

### Validation
- Build (Release x64) passed after change:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /p:Configuration=Release /p:Platform=x64 /m:1`

## 2026-02-21 Follow-up: plugin switch API could report false success

### Updated diagnosis
- Result: `Bug`.
- Symptom: user switched manifest in Web WASM panel, but runtime effect stayed on previous plugin.

### Root cause
- `POST /api/wasm/load-manifest` previously called `HandleCommand("wasm_load_manifest")` and then used `Diagnostics().pluginLoaded` as success flag.
- If a previous plugin was already loaded, `pluginLoaded` could remain `true` even when new manifest load failed.
- This produced false-positive `ok=true` responses, so UI looked successful while active plugin did not change.

### Fix
- `WebSettingsServer` load-manifest route now keeps UI-thread-safe command dispatch:
  - `controller_->HandleCommand({"cmd":"wasm_load_manifest", ...})`
  - then validates success by checking runtime diagnostics:
    - `plugin_loaded == true`
    - `active_manifest_path` matches requested path (normalized compare).
- If manifest does not actually become active, API returns:
  - `ok=false`
  - `error="manifest switch did not take effect"`
  - instead of false-positive success.
- `CommandHandler::HandleWasmLoadManifestCommand` now prefers full JSON parse (`nlohmann::json`) for `manifest_path`, with `JsonLite` fallback only if parse fails.

### Files (switch follow-up)
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/CommandHandler.cpp`

## 2026-02-21 Frontend follow-up: selector was reverting to active plugin

### Updated diagnosis
- Result: `Bug / regression`.
- Symptom: in Web WASM panel, selecting a different plugin in dropdown could revert to current active plugin before clicking `Load Selected`.
- Impact: user operation looked like "switched", but request still used old manifest path.

### Root cause
- Runtime bundle (`wasm-settings.svelte.js`) contained a reactive sync that continuously forced `selectedManifestPath` back to `active_manifest_path`.
- This created a selection lock and blocked real plugin switching from UI.

### Fix
- In `WasmPluginFields.svelte`:
  - removed continuous active-path override behavior
  - added normalized manifest-path matching helper
  - kept selection stable across catalog refresh
  - after successful `loadManifest`, performs one-shot catalog refresh and aligns selection to the newly active manifest.
- Rebuilt WASM section bundle and synced runtime web UI assets (`x64/Debug/webui`, `x64/Release/webui`).

### Files (frontend follow-up)
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- `x64/Debug/webui/wasm-settings.svelte.js` (generated runtime asset)
- `x64/Release/webui/wasm-settings.svelte.js` (generated runtime asset)
