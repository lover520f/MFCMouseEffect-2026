import {
  API_VERSION,
  EVENT_KIND_MOVE,
  FRAME_INPUT_BYTES,
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

const FILL_ARGB: u32 = 0x28FFF6F0;
const STROKE_ARGB: u32 = 0xFFFF786C;
const GLOW_ARGB: u32 = 0x66FF786C;

let gAnchorX: f32 = 0.0;
let gAnchorY: f32 = 0.0;
let gHasAnchor: bool = false;

function emitRing(outputPtr: usize, outputCap: u32, x: f32, y: f32, size: f32): u32 {
  if (outputCap < SPAWN_PULSE_COMMAND_BYTES * 2) {
    return 0;
  }

  writeSpawnPulse(
    outputPtr,
    x,
    y,
    size * 0.18,
    size,
    2.2,
    0.88,
    FILL_ARGB,
    STROKE_ARGB,
    GLOW_ARGB,
    0,
    150,
    PULSE_KIND_RIPPLE,
  );
  writeSpawnPulse(
    outputPtr + <usize>SPAWN_PULSE_COMMAND_BYTES,
    x,
    y,
    size * 0.10,
    size * 0.40,
    1.2,
    0.72,
    0x24FFFFFF,
    0xFFFFF2EA,
    GLOW_ARGB,
    0,
    120,
    PULSE_KIND_STAR,
  );
  return SPAWN_PULSE_COMMAND_BYTES * 2;
}

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
  if (!gHasAnchor) {
    return 0;
  }
  return emitRing(outputPtr, outputCap, gAnchorX, gAnchorY, 26.0);
}
