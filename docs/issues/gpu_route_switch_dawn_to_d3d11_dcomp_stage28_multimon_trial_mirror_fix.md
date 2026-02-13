# GPU Route Switch Stage 28: Multi-Monitor Trial Mirror Fix

Date: 2026-02-13

## Goal
Fix cross-monitor mirrored rendering artifact introduced by visible-trial frame uploads.

## Symptom
In multi-monitor setups, effects from monitor 2 could appear on monitor 1 (similar to duplicated/mirrored output).

## Root Cause
Visible-trial swapchain is bound to a single host hwnd, but `RenderSurface()` uploaded every monitor surface into that same swapchain each frame.

## Changes
- In `OverlayHostWindow::RenderSurface()`, trial frame upload is now limited to the bound host surface (`surface.hwnd == timerHwnd_`).

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - no cross-monitor mirrored trial output.
  - monitor-local effects remain local.

## Risk
- Low. Limits trial upload source selection only; authoritative layered present unchanged.
