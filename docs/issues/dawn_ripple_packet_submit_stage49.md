# Stage49: Ripple Packet Submit Entry

## Goal
- Split ripple (hold/hover) submission path from generic empty-command submit path.
- Prepare a dedicated entry for upcoming real ripple draw-pass migration.

## Changes
### 1) Runtime API
- Files:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added:
  - `TrySubmitRippleBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut)`
- Current behavior:
  - validates baked payload size
  - uses tagged empty command submit (`ripple_pass`) as stable transport
  - returns detailed diagnostics: `ripple_packet_submit_ok_v*_u*`

### 2) Consumer path split
- File:
  - `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- For non-trail ripple-only frames with baked data:
  - prefer `TrySubmitRippleBakedPacket(...)`
  - track dedicated counters:
    - `ripplePacketSubmitAttempts`
    - `ripplePacketSubmitSuccess`
- Existing trail/non-trail paths remain compatible.

### 3) API / UI diagnostics
- Files:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `MFCMouseEffect/WebUI/app.js`
- Exposed ripple packet counters in diagnostics payload and Web `?diag=1` text.

## Boundary Of This Stage
- This stage introduces dedicated ripple submit entry and metrics.
- It still uses empty-command transport under the hood for stability.
- Next stage should replace this transport with actual ripple render-pass commands.

