import {
  API_VERSION,
  EVENT_KIND_MOVE,
  PULSE_KIND_RIPPLE,
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

const FILL_ARGB: u32 = 0x42FFF5EB;
const CORE_ARGB: u32 = 0xCCFFECE0;
const STROKE_ARGB: u32 = 0xFFFFA082;
const GLOW_ARGB: u32 = 0x72FFB59C;

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

  writeSpawnPulse(outputPtr, gAnchorX, gAnchorY, 8.0, 22.0, 0.0, 0.92, CORE_ARGB, STROKE_ARGB, GLOW_ARGB, 0, 110, PULSE_KIND_RIPPLE);
  writeSpawnPulse(outputPtr + <usize>SPAWN_PULSE_COMMAND_BYTES, gAnchorX, gAnchorY, 14.0, 34.0, 0.0, 0.74, FILL_ARGB, STROKE_ARGB, GLOW_ARGB, 0, 140, PULSE_KIND_RIPPLE);
  writeSpawnPulse(outputPtr + <usize>SPAWN_PULSE_COMMAND_BYTES * 2, gAnchorX, gAnchorY, 20.0, 44.0, 0.0, 0.48, 0x10FFFFFF, STROKE_ARGB, GLOW_ARGB, 18, 190, PULSE_KIND_RIPPLE);
  return SPAWN_PULSE_COMMAND_BYTES * 3;
}
