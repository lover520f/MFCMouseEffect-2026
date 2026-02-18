# WASM Plugin Troubleshooting

This guide is for the current v1 route:
- runtime bridge: `mfx_wasm_runtime.dll`
- host diagnostics: `/api/state` -> `wasm`

## 1. Manifest load failed

Symptoms:
- `POST /api/wasm/load-manifest` returns `ok=false`
- `wasm.last_error` has manifest/module path text

Check:
- `manifest_path` is absolute and valid.
- `plugin.json` includes required fields.
- `entry` file exists and points to `effect.wasm`.

## 2. Runtime bridge load failed

Symptoms:
- `wasm.plugin_loaded=false`
- `wasm.last_error` includes dll/export message
- `wasm.runtime_backend="null"` and `wasm.runtime_fallback_reason` is non-empty

Check:
- Build from repo first: `MFCMouseEffect.slnx` (`x64 Debug/Release`) now produces `mfx_wasm_runtime.dll` automatically.
- `mfx_wasm_runtime.dll` exists beside `MFCMouseEffect.exe`
  or in process search path.
- bridge exports include:
  - `mfx_wasm_runtime_create`
  - `mfx_wasm_runtime_call_on_click`
  - `mfx_wasm_runtime_last_error`

## 3. Plugin loaded but no visible effect

Symptoms:
- `wasm.plugin_loaded=true`, but no visual output

Check:
- render-path hookup is enabled for `spawn_text` and `spawn_image`.
- validate runtime fields first:
  - `wasm.last_output_bytes`
  - `wasm.last_command_count`
  - `wasm.last_parse_error`
  - `wasm.last_rendered_by_wasm`
  - `wasm.last_executed_text_commands`
  - `wasm.last_executed_image_commands`
  - `wasm.last_render_error`
- if parser succeeds but still no visible output:
  - check whether current effect route is suppressed by focus/VM policy
  - inspect `wasm.last_budget_reason` and truncation flags

## 4. Budget rejection/truncation

Symptoms:
- `wasm.last_call_rejected_by_budget=true`
- `wasm.last_output_truncated_by_budget=true`
- `wasm.last_command_truncated_by_budget=true`

Reason field:
- `wasm.last_budget_reason`

Typical actions:
- reduce command count per event
- reduce output buffer usage
- simplify per-event logic cost

## 5. Parse errors

`wasm.last_parse_error` values:
- `truncated_header`
- `invalid_command_size`
- `truncated_command`
- `unsupported_command_kind`
- `command_limit_exceeded`

Action:
- ensure command binary layout matches `WasmPluginAbi.h`.
- ensure command `sizeBytes` exactly matches the emitted struct bytes.

## 6. Catalog is empty (`No plugins discovered`)

Symptoms:
- Web WASM section catalog is empty
- `/api/wasm/catalog` returns `count=0`

Check:
- Inspect `/api/wasm/catalog` `search_roots` to see actual scan directories.
- In Web settings, verify `Catalog root path` is set correctly (or clear it to use defaults).
- Ensure at least one valid `plugin.json` exists under those roots.
- In Debug-from-repo mode, host auto-scans `examples/wasm-plugin-template/dist`.
- If catalog is empty, `Reload Plugin` is intentionally disabled (no active plugin).

## 7. Quick self-check sequence

1. Build template (`examples/wasm-plugin-template`).
2. Copy `effect.wasm` + `plugin.json` to plugin folder.
3. Call `/api/wasm/load-manifest`.
4. Call `/api/wasm/enable`.
5. Click once, then inspect `/api/state` `wasm` diagnostics.

## 8. Web settings diagnostics panel

If you use Web settings WASM section:
- verify:
  - Last call metrics
  - Budget flags
  - Budget reason
  - Parse error
- warning-highlighted rows indicate budget/parse risks that can suppress output.
