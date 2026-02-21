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

## 2026-02-08 Update: Disconnection Banner Sync
- Issue: After initial successful load, the top-left status could stay at "Ready" even if the local server was later stopped or the connection dropped, which looked like a UI bug.
- Fix:
  - Added lightweight health probing in WebUI (`/api/state`, every 3 seconds, plus immediate probe when tab becomes visible).
  - Added an explicit `offline` state and banner text: "Disconnected from local server. Reopen from tray to continue."
  - After successful `Stop`, state switches to `stopped` immediately; service-dependent buttons stay clickable but disconnected actions now show a popup and are aborted.
  - Added a stronger visual banner style for disconnection (red warning tone) to clearly distinguish it from `Ready`.

## 2026-02-08 Update 2: Popup on Disconnected Actions
- Requirement: when users click actions like Apply while disconnected, show a clear popup to avoid "button does nothing" confusion.
- Implementation:
  - Added a shared guard `blockActionWhenDisconnected()`.
  - `Reload/Apply/Reset/Stop` now check connection state before sending requests.
  - If state is `offline/stopped/unauthorized`, show a blocking warning dialog and abort the action.
  - Top-left banner is still kept, so users get both persistent status and immediate click feedback.
  - Replaced native browser `alert` with a themed dialog (`dialog.svelte.js`) to match WebUI style.

## 2026-02-08 Update 3: Dialog Consistency + Localized Runtime Status
- Interaction consistency:
  - "Reset to defaults" confirmation now uses the themed confirm dialog instead of browser-native `confirm`.
  - Removed mixed usage of native/system dialogs and themed dialogs in the web settings flow.
- Text consistency:
  - Runtime status messages (`Loading`, `Applying`, `Reload failed`, etc.) are now localized and switch with UI language.

## 2026-02-21 Update: Prevent Active Tab from Becoming Stale on Reopen
- Problem:
  - Clicking tray **Settings...** while the web server was already running minted a new token every time.
  - Any already-open settings tab immediately became unauthorized, which could look like "buttons do nothing" (especially in WASM panel actions).
- Fix:
  - Keep the same token while the server is running.
  - Rotate token only when starting a new web server session.
- Result:
  - Reopening settings from tray no longer invalidates the currently open tab.
  - Unauthorized state is still possible after server restart, but not from same-session reopen.

## Manual Test Checklist
- Leave settings idle past the timeout, then open settings again -> no crash.
- Open settings twice within the same running session; both tabs should keep working.
- Apply settings from the newest tab; effects update immediately and `config.json` persists.
