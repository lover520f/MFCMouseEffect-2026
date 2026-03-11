import {
  API_VERSION,
  BLEND_MODE_ADD,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  SPAWN_QUAD_BATCH_FLAG_SCREEN_BLEND,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnQuadBatchCommandBytesWithSemanticsAndClip,
  writeSpawnQuadBatchHeaderWithSemanticsAndClip,
  writeSpawnQuadBatchItem,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const ITEM_COUNT: u32 = 6;
const COMMAND_BYTES: u32 = spawnQuadBatchCommandBytesWithSemanticsAndClip(ITEM_COUNT);
const MAX_COMMANDS: u32 = 2;
const TAU: f32 = <f32>6.283185307179586;
const QUAD_GROUP_ID: u32 = 1011;

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function quadTint(button: u8, index: u32): u32 {
  if (button == BUTTON_RIGHT) {
    if ((index & 1) == 0) return 0xFFFFC857;
    return 0xFFFF6F61;
  }
  if (button == BUTTON_MIDDLE) {
    if ((index & 1) == 0) return 0xFF6FE8FF;
    return 0xFF8B7DFF;
  }
  if ((index & 1) == 0) return 0xFF54F3FF;
  return 0xFFFF6DCE;
}

function atlasCoord(index: u32, axis: u32): f32 {
  const cell = <f32>(index & 1);
  const row = <f32>((index >> 1) & 1);
  if (axis == 0) return cell * <f32>0.5;
  if (axis == 1) return row * <f32>0.5;
  if (axis == 2) return (cell + <f32>1.0) * <f32>0.5;
  return (row + <f32>1.0) * <f32>0.5;
}

function emitBurst(
  outputPtr: usize,
  x: f32,
  y: f32,
  button: u8,
  seed: u32,
  delayMs: u32,
  lifeMs: u32,
  sortKey: i32,
): void {
  const clipWidthPx: f32 = 74.0 + <f32>((button == BUTTON_RIGHT) ? 22.0 : 12.0);
  const clipHeightPx: f32 = 98.0 + <f32>((button == BUTTON_MIDDLE) ? 18.0 : 0.0);
  writeSpawnQuadBatchHeaderWithSemanticsAndClip(
    outputPtr,
    ITEM_COUNT,
    delayMs,
    lifeMs,
    SPAWN_QUAD_BATCH_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    sortKey,
    QUAD_GROUP_ID,
    x - clipWidthPx * 0.5,
    y - clipHeightPx * 0.5,
    clipWidthPx,
    clipHeightPx,
  );

  for (let index: u32 = 0; index < ITEM_COUNT; index += 1) {
    const angle = (<f32>index / <f32>ITEM_COUNT) * TAU
      + <f32>signedFromSeed(seed, index * 17 + 3, 16) * <f32>0.017453292;
    const speed = <f32>90.0 + <f32>rangedFromSeed(seed, index * 23 + 11, 0, 120);
    const widthPx = <f32>22.0 + <f32>rangedFromSeed(seed, index * 29 + 7, 0, 34);
    const heightPx = <f32>18.0 + <f32>rangedFromSeed(seed, index * 31 + 9, 0, 28);
    const rotation = angle + <f32>signedFromSeed(seed, index * 37 + 13, 45) * <f32>0.017453292;
    const alpha = <f32>0.62 + <f32>rangedFromSeed(seed, index * 41 + 19, 0, 26) / <f32>100.0;
    const vx = <f32>Math.cos(angle) * speed;
    const vy = <f32>Math.sin(angle) * speed - <f32>36.0;
    const ax = <f32>signedFromSeed(seed, index * 43 + 23, 20);
    const ay = <f32>130.0 + <f32>rangedFromSeed(seed, index * 47 + 29, 0, 80);
    const jitterX = <f32>signedFromSeed(seed, index * 53 + 31, 8);
    const jitterY = <f32>signedFromSeed(seed, index * 59 + 37, 8);
    const cellIndex = (index + seed) & 3;
    writeSpawnQuadBatchItem(
      outputPtr,
      index,
      x + jitterX,
      y + jitterY,
      widthPx,
      heightPx,
      alpha,
      rotation,
      quadTint(button, index + seed),
      rangedFromSeed(seed, index * 61 + 41, 0, 2),
      atlasCoord(cellIndex, 0),
      atlasCoord(cellIndex, 1),
      atlasCoord(cellIndex, 2),
      atlasCoord(cellIndex, 3),
      vx,
      vy,
      ax,
      ay,
    );
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
  emitBurst(offset, x, y, button, seed ^ 0x4A9E13F1, 0, 520, 28);
  written += 1;
  offset += <usize>COMMAND_BYTES;

  if (written >= commandCap) {
    return written * COMMAND_BYTES;
  }

  emitBurst(offset, x, y - <f32>8.0, button, seed ^ 0xC15D702B, 42, 360, 14);
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
