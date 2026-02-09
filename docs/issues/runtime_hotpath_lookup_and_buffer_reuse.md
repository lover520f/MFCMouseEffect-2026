# Runtime Hot Path Optimization: ID Lookup + Buffer Reuse

## Goal
- Reduce CPU overhead in high-frequency update/render paths without changing effect visuals or behavior.

## 1) O(1) Ripple Instance Lookup

### Problem
- `RippleOverlayLayer` used linear scan (`FindById`) for:
  - `UpdatePosition(id, pt)`
  - `SendCommand(id, cmd, args)`
  - `Stop(id)`
- Under frequent hold/hover updates, this becomes unnecessary O(n) work per call.

### Change
- Added `idToIndex_` map in `RippleOverlayLayer`:
  - File: `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h`
- Replaced linear lookup with map lookup:
  - File: `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
- Updated instance removal logic to in-place swap-remove while keeping map indices consistent.

### Result
- Lookup complexity for active ripple by ID is now O(1) average.
- Better scalability when multiple continuous ripple instances coexist.

## 2) Renderer Scratch Buffer Reuse

### Problem
- Several renderers created temporary `std::vector<Gdiplus::PointF>` in inner render loops each frame.
- This causes repeat allocations and allocator pressure on hot paths.

### Change
- `HologramHudRenderer`
  - File: `MFCMouseEffect/MouseFx/Renderers/Hold/HologramHudRenderer.h`
  - Added `segmentPointsScratch_` member and reused it in segmented ring drawing.
  - Added `particles_.reserve(30)` in `Start(...)`.
- `TechRingRenderer`
  - File: `MFCMouseEffect/MouseFx/Renderers/Hold/TechRingRenderer.h`
  - Added `ringPointsScratch_` member and reused it in ring drawing.
  - Added `particles_.reserve(40)` in `Start(...)`.

### Result
- Lower per-frame temporary allocation frequency in hold renderers.
- Reduced allocator churn and more stable frame pacing on CPU fallback path.

## 3) Typed Hold Progress Fast Path

### Problem
- `HoldEffect` previously sent hold progress through string commands each update:
  - `snprintf(...)` in producer
  - `OnCommand + sscanf_s(...)` in renderer
- This path ran on high-frequency hold updates and introduced avoidable format/parse overhead.

### Change
- Added typed hold APIs in `IRippleRenderer`:
  - `SetHoldElapsedMs(uint32_t)`
  - `SetHoldDurationMs(uint32_t)`
  - File: `MFCMouseEffect/MouseFx/Interfaces/IRippleRenderer.h`
- Added typed forwarding in host/layer:
  - `OverlayHostService::UpdateRippleHoldElapsed/UpdateRippleHoldThreshold`
  - `RippleOverlayLayer::SendHoldElapsed/SendHoldThreshold`
  - Files:
    - `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
    - `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
    - `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h`
    - `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
- Switched `HoldEffect` to typed calls instead of per-frame string formatting:
  - File: `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
- `HoldNeon3DRenderer` now overrides typed methods to update state directly.
  - Keeps `OnCommand(...)` parsing for backward compatibility with legacy/external command paths.
  - File: `MFCMouseEffect/MouseFx/Renderers/Hold/HoldNeon3DRenderer.h`

### Result
- Removed string format/parse overhead from the primary hold update hot path.
- Kept compatibility for existing command-based integrations.

## Validation
- In this environment, `MSBuild.exe`/VC toolchain is unavailable in PATH, so full compile verification could not be executed.
- Attempted:
  - `dotnet build MFCMouseEffect/MFCMouseEffect.vcxproj -c Release -p:Platform=x64 -m:1`
- Result:
  - Fails with `MSB4278` (`Microsoft.Cpp.Default.props` not found in dotnet CLI context), which is expected for VC++ projects without VS C++ MSBuild toolchain.
