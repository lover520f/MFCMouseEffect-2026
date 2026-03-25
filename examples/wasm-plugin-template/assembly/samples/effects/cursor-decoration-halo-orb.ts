import {
  API_VERSION,
  EVENT_KIND_MOVE,
  PULSE_KIND_RIPPLE,
  PULSE_KIND_STAR,
  SPAWN_PULSE_COMMAND_BYTES,
  canHandleEvent,
  canHandleFrameInput,
  readEventKind,
  readEventX,
  readEventY,
  readFrameCursorX,
  readFrameCursorY,
  readFramePointerValid,
  writeSpawnPulse,
} from "../../common/abi";

const CORE_FILL_ARGB: u32 = 0x2EE7FAFF;
const CORE_STROKE_ARGB: u32 = 0xFF59D8FF;
const HALO_GLOW_ARGB: u32 = 0x7059D8FF;
const OUTER_STROKE_ARGB: u32 = 0xFF9CEBFF;

let gAnchorX: f32 = 0.0;
let gAnchorY: f32 = 0.0;
let gHasAnchor: bool = false;

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {
  gAnchorX = 0.0;
  gAnchorY = 0.0;
  gHasAnchor = false;
}

export function mfx_plugin_on_input(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  outputPtr;
  if (!canHandleEvent(inputLen, outputCap, SPAWN_PULSE_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_MOVE) {
    return 0;
  }
  gAnchorX = <f32>readEventX(inputPtr);
  gAnchorY = <f32>readEventY(inputPtr);
  gHasAnchor = true;
  return 0;
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleFrameInput(inputLen, outputCap, SPAWN_PULSE_COMMAND_BYTES)) {
    return 0;
  }
  if (readFramePointerValid(inputPtr)) {
    gAnchorX = <f32>readFrameCursorX(inputPtr);
    gAnchorY = <f32>readFrameCursorY(inputPtr);
    gHasAnchor = true;
  }
  if (!gHasAnchor || outputCap < SPAWN_PULSE_COMMAND_BYTES * 3) {
    return 0;
  }

  writeSpawnPulse(outputPtr, gAnchorX, gAnchorY, 6.0, 20.0, 0.0, 0.94, CORE_FILL_ARGB, CORE_STROKE_ARGB, HALO_GLOW_ARGB, 0, 110, PULSE_KIND_STAR);
  writeSpawnPulse(outputPtr + <usize>SPAWN_PULSE_COMMAND_BYTES, gAnchorX, gAnchorY, 14.0, 36.0, 1.3, 0.66, 0x18F5FDFF, OUTER_STROKE_ARGB, HALO_GLOW_ARGB, 0, 160, PULSE_KIND_RIPPLE);
  writeSpawnPulse(outputPtr + <usize>SPAWN_PULSE_COMMAND_BYTES * 2, gAnchorX, gAnchorY, 22.0, 50.0, 1.9, 0.34, 0x08FFFFFF, OUTER_STROKE_ARGB, HALO_GLOW_ARGB, 24, 220, PULSE_KIND_RIPPLE);
  return SPAWN_PULSE_COMMAND_BYTES * 3;
}
