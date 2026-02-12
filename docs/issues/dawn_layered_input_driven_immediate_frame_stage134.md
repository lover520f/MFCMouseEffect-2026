# Dawn Layered Input-Driven Immediate Frame (Stage 134)

## Background
- In current architecture, Dawn backend is active but final present is still layered CPU fallback.
- Layered path is frame-timer driven; input updates can wait until the next timer tick, increasing cursor-to-effect latency.

## Root Cause
- Render cadence depended on periodic timer (`WM_TIMER`) only.
- High-frequency pointer updates were not allowed to trigger a near-immediate host frame.

## Changes
1. Added `OverlayHostWindow::RequestImmediateFrame()`:
   - posts a dedicated immediate-frame message to host timer window;
   - includes a short minimum kick interval and in-flight coalescing to avoid flood.
2. Added `OverlayHostService::RequestImmediateFrame()` forwarding API.
3. Connected immediate-frame requests in `AppController` after effect-changing inputs:
   - click, move, scroll, hold start/end;
   - trail latency-priority mode switch.

## Expected Effect
- Reduces input-to-visual wait under layered fallback path.
- Improves hold/trail follow responsiveness without changing effect semantics.

## Safety
- Does not alter takeover gate or final-present mode decisions.
- Coalescing + min interval reduce message storm risk.
