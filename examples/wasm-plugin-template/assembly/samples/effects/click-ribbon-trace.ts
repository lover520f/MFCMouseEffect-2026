import {
  API_VERSION,
  BLEND_MODE_ADD,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  EVENT_KIND_CLICK,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnRibbonStripCommandBytesWithSemantics,
  writeSpawnRibbonStripHeaderWithSemantics,
  writeSpawnRibbonStripPoint,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const POINT_COUNT: u32 = 7;
const COMMAND_BYTES: u32 = spawnRibbonStripCommandBytesWithSemantics(POINT_COUNT);
const MAX_COMMANDS: u32 = 2;
const RIBBON_GROUP_ID: u32 = 1017;

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function colorFor(button: u8, layer: u32): u32 {
  if (button == BUTTON_MIDDLE) {
    return layer == 0 ? 0xFF73E7FF : 0xFF8D7AFF;
  }
  if (button == BUTTON_LEFT) {
    return layer == 0 ? 0xFFFF7DD0 : 0xFF4EE8FF;
  }
  return layer == 0 ? 0xFFFFC864 : 0xFFFF7A64;
}

function emitRibbon(
  outputPtr: usize,
  x: f32,
  y: f32,
  seed: u32,
  button: u8,
  delayMs: u32,
  lifeMs: u32,
  layer: u32,
  sortKey: i32,
): void {
  writeSpawnRibbonStripHeaderWithSemantics(
    outputPtr,
    POINT_COUNT,
    <f32>0.86,
    layer == 0 ? <f32>9.0 : <f32>6.0,
    colorFor(button, layer),
    colorFor(button, layer) & 0x00FFFFFF | 0x55000000,
    delayMs,
    lifeMs,
    0,
    BLEND_MODE_ADD,
    sortKey,
    RIBBON_GROUP_ID,
  );

  for (let index: u32 = 0; index < POINT_COUNT; index += 1) {
    const t = <f32>index / <f32>(POINT_COUNT - 1);
    const dx = t * <f32>132.0 - <f32>30.0;
    const wave = <f32>Math.sin(t * <f32>5.1 + <f32>layer * <f32>0.55) * <f32>28.0;
    const drift = <f32>signedFromSeed(seed, index * 19 + layer * 13 + 7, 8);
    const px = x + dx + drift;
    const py = y + wave - t * <f32>48.0 + <f32>signedFromSeed(seed, index * 23 + layer * 17 + 11, 6);
    const widthPx = (<f32>34.0 - t * <f32>20.0)
      + <f32>rangedFromSeed(seed, index * 29 + layer * 5 + 3, 0, 5);
    writeSpawnRibbonStripPoint(outputPtr, index, px, py, widthPx);
  }
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
  const commandCap = minU32(outputCap / COMMAND_BYTES, MAX_COMMANDS);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  let written: u32 = 0;
  emitRibbon(offset, x, y, seed ^ 0x51A7F19B, button, 0, 420, 0, 26);
  written += 1;
  offset += <usize>COMMAND_BYTES;
  if (written >= commandCap) {
    return written * COMMAND_BYTES;
  }

  emitRibbon(offset, x + <f32>8.0, y - <f32>10.0, seed ^ 0xC4072D61, button, 46, 320, 1, 14);
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
