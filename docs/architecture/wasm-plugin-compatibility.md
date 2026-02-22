# WASM Plugin Compatibility Policy

This policy defines forward/backward rules for `MFCMouseEffect` WASM plugins.

## 1. Version markers

- Host contract version: `api_version` in `plugin.json`
- Runtime ABI entry:
  - `mfx_plugin_get_api_version()`
  - currently expected value: `1`

Both values must match host-supported ABI major version.

## 2. ABI compatibility rules

For ABI v1:
- Existing exported function names must stay stable.
- Existing binary struct field order must stay stable.
- New fields are append-only.
- Existing enum values are immutable.
- `mfx_plugin_on_event` is the required event entry for the current host.

Breaking layout/function changes require ABI major bump (v2+).

## 3. Manifest compatibility rules

`plugin.json` required fields:
- `id` (stable plugin identity)
- `name`
- `version`
- `api_version`
- `entry` (wasm module relative path)

Rules:
- `id` must remain stable across plugin upgrades.
- `entry` path must stay within plugin folder.
- Unknown fields are ignored by host, so plugin authors may add private metadata.

## 4. Runtime fallback behavior

If runtime bridge cannot be loaded:
- host uses `Null` runtime fallback.
- plugin stays non-functional but app remains stable.

If plugin call exceeds budget:
- current event output may be dropped/truncated.
- diagnostics are recorded in `/api/state` `wasm` block.

## 5. Upgrade guidance for plugin authors

- Keep one source branch per ABI major version (`v1`, `v2`).
- Pin template baseline in repository tags/releases.
- Rebuild plugin when host ABI major changes.
- Verify with:
  - `POST /api/wasm/load-manifest`
  - `POST /api/wasm/enable`
  - `GET /api/state` (`wasm.plugin_api_version`, `wasm.last_error`)

## 6. Host-side compatibility commitment (current)

- v1 host accepts ABI v1 plugins only.
- Host may add diagnostics and optional manifest metadata without breaking v1 plugins.
