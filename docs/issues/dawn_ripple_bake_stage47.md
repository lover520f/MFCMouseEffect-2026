# Stage47: Ripple Vertex Bake For Future DrawPass

## Goal
- Move ripple (hold/hover) path from simple budget counters to concrete vertex-bake preparation.
- Keep current queue submit stability while preparing direct Dawn draw-pass integration.

## Changes
### 1) New ripple bake preprocessor
- Added:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRippleGeometryPreprocessor.h`
- For each `RipplePulse` vertex, build a screen-space quad (2 triangles, 6 vertices) into scratch buffer.
- Output now includes:
  - `bakedQuads`
  - `bakedVertices`
  - upload bytes based on baked vertex payload.

### 2) Trail preprocessor composes ripple preprocessor
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnTrailGeometryPreprocessor.h`
- Ripple preprocess logic is no longer embedded there.
- Trail preprocessor now aggregates ripple bake metrics into unified prep result.

### 3) Consumer diagnostics upgraded
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- Added fields:
  - `preparedRippleBakedQuads`
  - `preparedRippleBakedVertices`
- Non-trail submit detail appends `_ripple_baked` when baked ripple vertices exist.

### 4) API / Web sync
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `MFCMouseEffect/WebUI/app.js`
- Added ripple bake counters to diagnostics payload and UI text.

## Boundary Of This Stage
- This stage prepares draw-ready ripple geometry data on CPU side.
- It does **not** yet bind a Dawn render pass / pipeline / target for ripple rendering.
- Next stage should connect baked ripple buffers into actual draw calls.

