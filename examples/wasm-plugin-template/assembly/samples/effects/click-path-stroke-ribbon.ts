import {
  API_VERSION,
  BLEND_MODE_ADD,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  PATH_STROKE_LINE_CAP_ROUND,
  PATH_STROKE_LINE_JOIN_ROUND,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnPathStrokeCommandBytesWithSemantics,
  writePathStrokeNodeClose,
  writePathStrokeNodeCubicTo,
  writePathStrokeNodeLineTo,
  writePathStrokeNodeMoveTo,
  writePathStrokeNodeQuadTo,
  writeSpawnPathStrokeHeaderWithSemantics,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const PRIMARY_NODE_COUNT: u32 = 5;
const ACCENT_NODE_COUNT: u32 = 4;
const PRIMARY_COMMAND_BYTES: u32 = spawnPathStrokeCommandBytesWithSemantics(PRIMARY_NODE_COUNT);
const ACCENT_COMMAND_BYTES: u32 = spawnPathStrokeCommandBytesWithSemantics(ACCENT_NODE_COUNT);

function strokeColor(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFF8A48;
  if (button == BUTTON_MIDDLE) return 0xFFF4F1FF;
  return 0xFF42F5D7;
}

function glowColor(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0x66FF8A48;
  if (button == BUTTON_MIDDLE) return 0x88A77BFF;
  return 0x6637D8FF;
}

function blendMode(button: u8): u32 {
  return button == BUTTON_MIDDLE ? BLEND_MODE_ADD : BLEND_MODE_SCREEN;
}

function emitPrimaryRibbon(
  outputPtr: usize,
  x: f32,
  y: f32,
  direction: f32,
  seed: u32,
  button: u8,
): void {
  const drift: f32 = <f32>signedFromSeed(seed, 3, 6);
  const lift: f32 = 28.0 + <f32>rangedFromSeed(seed, 7, 0, 16);
  const reach: f32 = 40.0 + <f32>rangedFromSeed(seed, 11, 0, 20);
  const tail: f32 = 18.0 + <f32>rangedFromSeed(seed, 15, 0, 14);

  writeSpawnPathStrokeHeaderWithSemantics(
    outputPtr,
    PRIMARY_NODE_COUNT,
    button == BUTTON_MIDDLE ? 4.2 : 3.4,
    0.96,
    strokeColor(button),
    glowColor(button),
    0,
    320,
    PATH_STROKE_LINE_JOIN_ROUND,
    PATH_STROKE_LINE_CAP_ROUND,
    blendMode(button),
    28,
    1,
  );

  writePathStrokeNodeMoveTo(outputPtr, 0, x - direction * tail, y + 16.0 + drift);
  writePathStrokeNodeCubicTo(
    outputPtr,
    1,
    x - direction * 2.0,
    y - lift * 0.52,
    x + direction * (reach * 0.56),
    y - lift * 1.08,
    x + direction * reach,
    y - lift + drift * 0.4,
  );
  writePathStrokeNodeQuadTo(
    outputPtr,
    2,
    x + direction * (reach * 0.52),
    y + 6.0,
    x + direction * 10.0,
    y + 4.0,
  );
  writePathStrokeNodeLineTo(outputPtr, 3, x - direction * 12.0, y + 22.0 + drift * 0.5);
  writePathStrokeNodeClose(outputPtr, 4);
}

function emitAccentRibbon(
  outputPtr: usize,
  x: f32,
  y: f32,
  direction: f32,
  seed: u32,
  button: u8,
): void {
  const sway: f32 = <f32>signedFromSeed(seed, 19, 10);
  const crest: f32 = 18.0 + <f32>rangedFromSeed(seed, 23, 0, 14);
  const sweep: f32 = 34.0 + <f32>rangedFromSeed(seed, 29, 0, 18);

  writeSpawnPathStrokeHeaderWithSemantics(
    outputPtr,
    ACCENT_NODE_COUNT,
    button == BUTTON_RIGHT ? 1.8 : 1.6,
    0.62,
    button == BUTTON_MIDDLE ? 0xFFFFFFFF : 0xFFF7F4FF,
    glowColor(button),
    34,
    240,
    PATH_STROKE_LINE_JOIN_ROUND,
    PATH_STROKE_LINE_CAP_ROUND,
    blendMode(button),
    12,
    1,
  );

  writePathStrokeNodeMoveTo(outputPtr, 0, x - direction * 26.0, y - 8.0 + sway * 0.2);
  writePathStrokeNodeQuadTo(
    outputPtr,
    1,
    x + direction * 4.0,
    y - crest,
    x + direction * 24.0,
    y - 16.0,
  );
  writePathStrokeNodeLineTo(outputPtr, 2, x + direction * sweep, y - 6.0 + sway * 0.15);
  writePathStrokeNodeCubicTo(
    outputPtr,
    3,
    x + direction * (sweep + 8.0),
    y + 6.0,
    x + direction * 22.0,
    y + 22.0,
    x - direction * 4.0,
    y + 16.0 + sway * 0.25,
  );
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
  if (!canHandleEvent(inputLen, outputCap, PRIMARY_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const button = readEventButton(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  let direction: f32 = button == BUTTON_RIGHT ? -1.0 : 1.0;
  if (button == BUTTON_MIDDLE && signedFromSeed(seed, 31, 1) < 0) {
    direction = -1.0;
  }

  let written: u32 = 0;
  emitPrimaryRibbon(outputPtr, x, y, direction, seed ^ 0x9E3779B9, button);
  written += PRIMARY_COMMAND_BYTES;

  if (outputCap < written + ACCENT_COMMAND_BYTES) {
    return written;
  }

  emitAccentRibbon(
    outputPtr + <usize>written,
    x + direction * 6.0,
    y - 2.0,
    direction,
    seed ^ 0x7F4A7C15,
    button,
  );
  written += ACCENT_COMMAND_BYTES;
  return written;
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
