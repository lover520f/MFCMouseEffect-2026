# Web Settings Server Lifecycle: Idle Restart Crash + Token Rotation (2026-02-04)

## Symptoms
- After migrating settings to the browser UI, sometimes clicking tray **Settings...** makes the effect process exit.
- Opening settings multiple times keeps **all old tokens valid**; expected behavior is **only the latest token works**.

## Repro (Typical)
1. Tray -> **Settings...** to start the web server.
2. Leave it idle until the server auto-stops (idle timeout).
3. Click tray **Settings...** again -> intermittent crash.
4. Open settings multiple times -> all tabs still authorized.

## Root Causes
1. **Monitor thread not joined on idle stop**
   - `StartMonitor()` spawns `monitorThread_`.
   - On idle timeout, the monitor thread calls `http_->Stop()` and exits, **but the thread object remains joinable**.
   - The next `StartMonitor()` assigns a new `std::thread` to `monitorThread_` while it is still joinable, which triggers `std::terminate()` -> process exit.
2. **Token never rotated**
   - `token_` was generated once in the constructor and reused forever, so every settings URL stayed valid.
3. **Potential cross-thread config read**
   - `WebSettingsServer` read `AppController::config_` directly from the HTTP thread. That is a data race with UI-thread mutations and can lead to undefined behavior.

## Fix
- Join any previous `monitorThread_` before starting a new one.
- Add `AppController::GetConfigSnapshot()` (dispatch-thread copy via `WM_MFX_GET_CONFIG`) and use it in web server JSON builders.
- Add a token mutex + `RotateToken()`; **rotate on every tray Settings open** so only the latest token is valid.

## Behavior Changes
- Every time tray **Settings...** is clicked, a new token is minted.
- Old tabs will show `unauthorized` on API calls and should be reopened from the tray.
- Status messages (Ready / Server stopped / Token expired / Errors) now show only in a top-left banner (the bottom toast is removed).

## Manual Test Checklist
- Leave settings idle past the timeout, then open settings again -> no crash.
- Open settings twice; the first tab should fail API calls (unauthorized), the newest should work.
- Apply settings from the newest tab; effects update immediately and `config.json` persists.
