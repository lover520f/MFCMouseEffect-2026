# Server Structure Map (P2)

## Purpose
Detailed server layout notes for WebSettings and route layering.
P1 (`docs/agent-context/current.md`) keeps only a brief summary.

## Layer Map
- `MouseFx/Server/core/`: web settings server lifecycle + request routing/token monitor entry.
- `MouseFx/Server/routes/automation/`: automation + test-automation route handlers.
- `MouseFx/Server/routes/core/`: core API route + request gateway.
- `MouseFx/Server/routes/testing/`: test-only route handlers.
- `MouseFx/Server/routes/wasm/`: wasm catalog/runtime/import/export route handlers and wasm route utils.
- `MouseFx/Server/diagnostics/`: settings diagnostics mapping builders.
- `MouseFx/Server/settings/`: settings schema/state/wasm capabilities mapping.
- `MouseFx/Server/http/`: embedded http server implementation.
- `MouseFx/Server/webui/`: webui assets + webui path resolver.

## Include Boundary Notes
- Compatibility wrapper headers under `MouseFx/Server` are removed; includes point to concrete sub-layer paths.
- POSIX shell include boundary tightened:
  - `PosixCoreAppShell.h` forward-declares `mousefx::AppController` / `mousefx::WebSettingsServer`.
  - concrete headers live in `.cpp` or action units only.
- Scaffold POSIX headers trimmed:
  - `ScaffoldSettingsApi.h` no longer drags `HttpServer.h` or route config includes.
  - `ScaffoldSettingsRequestHandler.h` no longer drags `HttpServer.h`.
  - `ScaffoldSettingsRuntime.Internal.h` forward-declares `HttpServer` / `SettingsRequestHandler`.
