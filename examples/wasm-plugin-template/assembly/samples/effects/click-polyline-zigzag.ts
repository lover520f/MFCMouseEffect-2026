import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnPolylineCommandBytes,
  writeSpawnPolylineHeader,
  writeSpawnPolylinePoint,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const POINTS_PER_COMMAND: u32 = 5;
const COMMAND_BYTES: u32 = spawnPolylineCommandBytes(POINTS_PER_COMMAND);
const MAX_COMMANDS: u32 = 3;

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function strokeColor(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFF8A00;
  if (button == BUTTON_MIDDLE) return 0xFF7B61FF;
  return 0xFF31D7FF;
}

function glowColor(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0x66FF8A00;
  if (button == BUTTON_MIDDLE) return 0x667B61FF;
  return 0x6631D7FF;
}

function emitBolt(
  outputPtr: usize,
  x: f32,
  y: f32,
  strokeArgb: u32,
  glowArgb: u32,
  seed: u32,
  direction: f32,
  delayMs: u32,
  lifeMs: u32,
  lineWidthPx: f32,
  alpha: f32,
): void {
  writeSpawnPolylineHeader(
    outputPtr,
    POINTS_PER_COMMAND,
    lineWidthPx,
    alpha,
    strokeArgb,
    glowArgb,
    delayMs,
    lifeMs,
    0,
  );

  const span: f32 = <f32>24.0 + <f32>rangedFromSeed(seed, 2, 0, 18);
  const rise: f32 = <f32>18.0 + <f32>rangedFromSeed(seed, 5, 0, 22);
  const tail: f32 = <f32>12.0 + <f32>rangedFromSeed(seed, 8, 0, 12);
  const x0: f32 = x - direction * tail;
  const y0: f32 = y + <f32>signedFromSeed(seed, 11, 5);
  const x1: f32 = x + direction * <f32>6.0;
  const y1: f32 = y - rise * <f32>0.28 + <f32>signedFromSeed(seed, 14, 8);
  const x2: f32 = x - direction * (span * <f32>0.18);
  const y2: f32 = y - rise * <f32>0.58 + <f32>signedFromSeed(seed, 17, 9);
  const x3: f32 = x + direction * (span * <f32>0.46);
  const y3: f32 = y - rise * <f32>0.84 + <f32>signedFromSeed(seed, 20, 10);
  const x4: f32 = x + direction * span;
  const y4: f32 = y - rise - <f32>rangedFromSeed(seed, 23, 0, 14);

  writeSpawnPolylinePoint(outputPtr, 0, x0, y0);
  writeSpawnPolylinePoint(outputPtr, 1, x1, y1);
  writeSpawnPolylinePoint(outputPtr, 2, x2, y2);
  writeSpawnPolylinePoint(outputPtr, 3, x3, y3);
  writeSpawnPolylinePoint(outputPtr, 4, x4, y4);
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {}

export function mfx_plugin_on_input(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleEvent(inputLen, outputCap, COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const button = readEventButton(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const strokeArgb = strokeColor(button);
  const glowArgb = glowColor(button);
  const commandCap = minU32(outputCap / COMMAND_BYTES, MAX_COMMANDS);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  let written: u32 = 0;
  let direction: f32 = 1.0;
  if (button == BUTTON_RIGHT) {
    direction = -1.0;
  } else if (button == BUTTON_MIDDLE) {
    direction = <f32>(signedFromSeed(seed, 3, 1) == 0 ? 1 : -1);
  }

  emitBolt(
    offset,
    x,
    y,
    strokeArgb,
    glowArgb,
    seed ^ 0x9E3779B9,
    direction,
    0,
    240,
    2.8,
    0.98,
  );
  offset += <usize>COMMAND_BYTES;
  written += 1;

  if (written >= commandCap) {
    return written * COMMAND_BYTES;
  }

  emitBolt(
    offset,
    x + direction * 4.0,
    y - 3.0,
    strokeArgb,
    glowArgb,
    seed ^ 0x7F4A7C15,
    direction * -1.0,
    28,
    280,
    2.0,
    0.72,
  );
  offset += <usize>COMMAND_BYTES;
  written += 1;

  if (written >= commandCap) {
    return written * COMMAND_BYTES;
  }

  emitBolt(
    offset,
    x,
    y + 6.0,
    0xFFFFFFFF,
    glowArgb,
    seed ^ 0xA341316C,
    direction,
    54,
    160,
    1.2,
    0.48,
  );
  written += 1;
  return written * COMMAND_BYTES;
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  inputPtr;
  inputLen;
  outputPtr;
  outputCap;
  return 0;
}
