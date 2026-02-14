# Phase 4: AppController Decomposition

## Summary

Extracted three modules from `AppController.cpp` (1086 → ~545 lines, **50% reduction**), each with a single clear responsibility.

## Changes

### New Files

| File | Lines | Responsibility |
|------|-------|---------------|
| `Core/CommandHandler.h/cpp` | ~260 | JSON command routing + `apply_settings` payload parsing |
| `Core/GpuProbeHelper.h/cpp` | ~65 | Dawn WebGPU runtime probe + filesystem helpers |
| `Core/DispatchRouter.h/cpp` | ~250 | Win32 message dispatch (per-message handler methods) |

### Modified Files

| File | Change |
|------|--------|
| `Core/AppController.h` | Forward declarations, `friend class DispatchRouter`, 4 methods moved public, 2 new `unique_ptr` members |
| `Core/AppController.cpp` | `HandleCommand` → delegates to `CommandHandler`, `OnDispatchMessage` → delegates to `DispatchRouter`, GPU probe code replaced with `#include "GpuProbeHelper.h"` |
| `MFCMouseEffect.vcxproj` | Added 3 new `.cpp` entries |

## Design Decisions

- **CommandHandler** uses only public `AppController` methods → no friend needed
- **DispatchRouter** accesses private state fields (`holdButtonDown_`, `hovering_`, etc.) → `friend class` chosen for minimal disruption
- **GpuProbeHelper** is a free-function module (not a class) — matches the stateless, probe-once utility nature

## Build Verification

- `MSBuild Release|x64` → **0 errors, 0 warnings** (C4819 encoding note only)
