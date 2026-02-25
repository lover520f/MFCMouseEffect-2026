# Phase 55zza: macOS Dispatch Message Host Impl Split

## Capability
- Input dispatch mainline (shared foundation for effect/indicator/automation routing)

## Why
- `MacosDispatchMessageHost.cpp` mixed three responsibilities:
  - host lifecycle
  - message queue/sync dispatch worker
  - timer-thread management
- This increased coupling and made worker/timer changes harder to review.

## Scope
- Keep dispatch/timer behavior unchanged.
- Split implementation by responsibility into dedicated units.
- Keep class contract and external call paths unchanged.

## Code Changes

### 1) Header-level shared constants
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Control/MacosDispatchMessageHost.h`
- Moved error constants into class-level static constexpr fields for multi-translation-unit reuse.

### 2) Lifecycle-only base implementation
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Control/MacosDispatchMessageHost.cpp`
- Keeps:
  - constructor/destructor
  - `Create/Destroy`
  - `IsCreated/IsOwnerThread/NativeHandle/LastError`

### 3) Messaging worker implementation
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Control/MacosDispatchMessageHost.Messaging.cpp`
- Owns:
  - `SendSync/PostAsync`
  - `WorkerLoop`
  - `DispatchMessageOnWorker`
  - `EnqueueMessage`

### 4) Timer implementation
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Control/MacosDispatchMessageHost.Timers.cpp`
- Owns:
  - `SetTimer/KillTimer`
  - `StopAllTimers`

### 5) Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API/schema/user-visible behavior change.
- Internal structure only, reducing coupling risk in macOS dispatch host evolution.
