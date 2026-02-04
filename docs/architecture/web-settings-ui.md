# Web Settings UI (Loopback HTTP)

## Why
The MFC settings UI is hard to keep pretty and low-coupled, especially for “advanced tuning” (trail params).  
This project now prefers a **lightweight browser UI** served from a **local loopback HTTP server** to:
- Reduce UI/renderer coupling
- Make parameters easily extendable (schema/state API)
- Avoid MFC layout/hiDPI pain for dense tuning forms

## User Flow
1. Tray menu → **Settings...**
2. App starts a local server on `127.0.0.1` with an ephemeral port
3. App opens the default browser to `http://127.0.0.1:<port>/?token=<token>`
4. The page reads/writes settings via `/api/*` and applies instantly

## Apply Flow
- Changes are **manual apply**: update values then click **Apply**.
- **Reload** re-reads `config.json` from disk.
- **Reset** restores defaults (then reloads).
- **Stop** closes the local server (reopen from tray to start again).
## UI Hints
- Hover tips for top actions (Reload / Apply / Star).
- Text content clarifies using the English comma.
- Trail tuning includes **Idle Fade Start/End (ms)** to control convergence speed after mouse stops.
- Status banner at top-left shows Ready / Stopped / Token expired / Error states (bottom toast removed).

## Security Notes
- The server only binds to **loopback** (`127.0.0.1`).
- All `/api/*` endpoints require header `X-MFCMouseEffect-Token` matching the token in the URL query.
  - This is mainly to reduce loopback CSRF risk (any webpage could try calling `http://127.0.0.1:<port>/api/*`).
- The token rotates on each tray **Settings...** open; only the newest token is accepted.

## Packaging / Hot Update (Disk Override)
- Source UI lives in `MFCMouseEffect/WebUI/`.
- Build copies it to `$(OutDir)\webui\` (e.g. `x64\Release\webui\`).
- The server always prefers disk assets first, so editing files under `webui\` and refreshing the browser updates UI without rebuilding.
- If `webui\` is missing, the server falls back to embedded RCDATA for core files.

## Endpoints
- `GET /` → static UI (`/index.html`) served from `$(OutDir)\webui\` (disk override), or fallback RCDATA
- `GET /app.js`, `GET /styles.css` → static UI assets (same rule)
- `GET /api/schema` → option lists (themes, effect types, etc.)
- `GET /api/state` → current settings (theme/language/active effects + trail tuning)
- `POST /api/state` → apply settings (maps to `{"cmd":"apply_settings","payload":...}`)
- `POST /api/reload` → reload `config.json` from disk (maps to `{"cmd":"reload_config"}`)
- `POST /api/reset` → restore defaults (maps to `{"cmd":"reset_config"}`)
- `POST /api/stop` → stop local server (on-demand)

## Resource Use
- The server thread blocks on `accept()` (no busy polling).
- It also auto-stops after a short idle timeout to reduce background footprint.

## Implementation Pointers
- Server: `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- HTTP loop: `MFCMouseEffect/MouseFx/Server/HttpServer.cpp`
- Static files: `MFCMouseEffect/WebUI/` (copied to `$(OutDir)\webui\` by PostBuild)
- Embedded fallback: `MFCMouseEffect/res/MFCMouseEffect.rc2` (RCDATA)
- Apply handler: `MFCMouseEffect/MouseFx/Core/AppController.cpp` (`cmd == "apply_settings"`)

## Manual Test Checklist
- Tray → Settings opens browser and loads schema/state
- Save & Apply updates:
  - language/theme
  - active effects (click/trail/scroll/hold/hover)
  - text content (comma-separated list)
  - trail tuning (profiles + renderer params)
- Reload button reloads from disk and refreshes UI

## Troubleshooting
- **Dropdowns are empty / page shows “Load failed”:** open DevTools Network and check `/api/schema` and `/api/state` responses.
  - Unauthorized → URL missing `?token=...` or token header not sent.
  - 500 → server now returns a concrete error string; paste it into an issue.
- **Token expired:** the UI shows a toast hint; reopen from tray to get a fresh token.
- **Encoding error (`invalid UTF-8 byte`)** → server now sanitizes strings by validating UTF-8 and converting from ACP if needed.
