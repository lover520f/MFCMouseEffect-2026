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

## Validation
- Compiled with:
  - `MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:ClCompile /p:Configuration=Release /p:Platform=x64 /m:1`
- Compile passed with no new errors.
