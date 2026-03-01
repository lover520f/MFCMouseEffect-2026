# VM Foreground Effect Suppression

## Summary
- Problem: when the cursor enters a virtual machine window, mouse effects can still render at the host-entry position, causing visual mismatch.
- Goal: automatically suspend all mouse effects while a VM window is in foreground, and resume when focus returns to normal host apps.

## Root Cause
- Effect dispatch was only gated by low-level mouse events.
- No foreground-context guard existed for VM-hosted input capture scenarios.

## Implementation
- Added a dedicated detector: `MFCMouseEffect/MouseFx/Core/VmForegroundDetector.h`.
- Detection source:
  - foreground process name
  - foreground window class name
  - foreground window title
- VM token matching includes common identifiers such as:
  - `vmware`, `virtualbox`, `vmconnect`, `qemu`, `parallels`, `hyper-v`
- Added a check throttle (default `800ms`) to reduce per-event overhead.
- Check throttle supports test override via env:
  - `MFX_VM_FOREGROUND_SUPPRESSION_CHECK_INTERVAL_MS`
  - valid range: `10..5000` (out-of-range/invalid values fall back to default `800ms`)

## AppController Integration
- `AppController` now maintains VM suppression state.
- On entering VM foreground:
  - cancel hold timer and pending hold state
  - clear hold/hover transient runtime state
  - shutdown all active effect instances
  - skip click/move/scroll/hold/hover dispatch
- On leaving VM foreground:
  - re-initialize active effect instances
  - resume normal event dispatch
- `SetEffect(...)` now respects suppression state and avoids initializing effects during VM-suppressed periods.

## Validation
1. Start app on host desktop: effects render as usual.
2. Move/focus into VMware/VirtualBox/Hyper-V VM window: effects stop.
3. Return focus to host app: effects resume automatically.
4. Hold/hover should not remain "stuck" after VM entry/exit transitions.
