# Tray Right-Click Access Violation Fix

## Symptom
- Rare crash when right-clicking tray icon.
- Crash reported in `ucrtbase.dll` with access violation while building popup menu.

## Root Cause
- Tray menu check-state logic queried current effect via runtime object pointer:
  - `AppController::GetEffect(...) -> effect->TypeName()`.
- During effect switching/recreation windows, pointer lifetime could race with menu-building read, causing invalid pointer access.

## Fix
- Switched tray current-type lookup to config snapshot path:
  - `AppController::GetConfigSnapshot().active.{click|trail|scroll|hold|hover}`.
- This avoids dereferencing live effect instances in tray menu build path.

## File
- `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`

## Impact
- Eliminates tray-right-click pointer race in checkmark resolution.
- No behavior change to command mapping or menu content.

## 2026-02-09 Additional Hardening

### Symptom (still reported)
- Some environments still crashed at `TrayMenuBuilder.cpp` while executing:
  - `sub.AppendMenu(MF_STRING, it.trayCmd, label);`
- Debugger showed AV in `ucrtbase.dll` during tray menu build.

### Hardening Changes
- Added strict defensive guards in `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`:
  - Validate parent menu handle with `IsMenu(...)`.
  - Validate metadata pointer/count (`opts != nullptr`, `count > 0`, bounded upper limit).
  - Validate submenu creation result (`CreatePopupMenu()`).
  - Skip invalid command IDs (`trayCmd == 0`).
  - Use safe fallback label when metadata display text is empty.
  - Check all `AppendMenu(...)` return values; if parent append fails, destroy detached submenu handle to avoid leaks.
  - Use explicit flags (`MF_BYPOSITION` / `MF_BYCOMMAND`) for clarity and predictable behavior.
- Changed `AppendEffectSubMenu` title parameter from raw `const TCHAR*` to `const CString&` to avoid temporary-pointer ambiguity at call sites.

### Validation
- Rebuilt `Release|x64` successfully using:
  - `MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /p:Configuration=Release /p:Platform=x64 /m:1`
- Result: build succeeded with no errors.

## 2026-02-09 Follow-up: Crash still at `PickLabel(option.displayZh, ...)`

### New Observation
- Crash could still occur while reading menu text pointers from metadata:
  - `CString label = PickLabel(option.displayZh, option.displayEn, zh);`
- This indicates the menu path can observe corrupted metadata memory and should not dereference UI label pointers directly.

### Final Mitigation in Tray Path
- In tray menu build, label rendering no longer reads `EffectOption::displayZh/displayEn`.
- Tray labels now come from a command-id based fallback table (`FallbackOptionLabel(cmd, zh)`), which is static and deterministic.
- Metadata is only used to obtain ordering and command IDs; command ID read is guarded.
- Current-item checkmark logic is also command-id based (`IsCurrentTypeMatchByCommand`), no dependency on metadata text/value pointers.
- IPC JSON mapping switched to direct command-id switch (`TryBuildEffectJsonByCommand`) to avoid metadata pointer reads in command handling as well.

### Tradeoff
- Duplicated label/value mapping in tray module (and metadata remains in settings module), but tray stability is prioritized.
- This is intentionally defensive: if runtime memory is partially corrupted, tray right-click remains best-effort and avoids immediate process crash.
