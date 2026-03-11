import {
  API_VERSION,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  SPAWN_GLOW_BATCH_FLAG_SCREEN_BLEND,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnGlowBatchCommandBytesWithSemantics,
  writeSpawnGlowBatchHeaderWithSemantics,
  writeSpawnGlowBatchItem,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const ITEM_COUNT: u32 = 27;
const COMMAND_BYTES: u32 = spawnGlowBatchCommandBytesWithSemantics(ITEM_COUNT);
const MAX_COMMANDS: u32 = 2;
const TAU: f32 = <f32>6.283185307179586;
const GLOW_GROUP_ID: u32 = 1001;

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function palette(index: u32, button: u8): u32 {
  const normalized = index % 6;
  if (button == BUTTON_RIGHT) {
    if (normalized == 0) return 0xFFFFB300;
    if (normalized == 1) return 0xFFFF7043;
    if (normalized == 2) return 0xFFFF5252;
    if (normalized == 3) return 0xFFFFF176;
    if (normalized == 4) return 0xFFFF8A65;
    return 0xFFFFD54F;
  }
  if (button == BUTTON_MIDDLE) {
    if (normalized == 0) return 0xFF7C4DFF;
    if (normalized == 1) return 0xFF00B8D4;
    if (normalized == 2) return 0xFF536DFE;
    if (normalized == 3) return 0xFF18FFFF;
    if (normalized == 4) return 0xFFB388FF;
    return 0xFF64FFDA;
  }
  if (normalized == 0) return 0xFF31D7FF;
  if (normalized == 1) return 0xFFFF5ACD;
  if (normalized == 2) return 0xFF7BFF6A;
  if (normalized == 3) return 0xFFFFD94A;
  if (normalized == 4) return 0xFF4D7BFF;
  return 0xFFFF8F5A;
}

function emitBurst(
  outputPtr: usize,
  x: f32,
  y: f32,
  seed: u32,
  button: u8,
  delayMs: u32,
  lifeMs: u32,
  energyScale: f32,
  sortKey: i32,
): void {
  writeSpawnGlowBatchHeaderWithSemantics(
    outputPtr,
    ITEM_COUNT,
    delayMs,
    lifeMs,
    SPAWN_GLOW_BATCH_FLAG_SCREEN_BLEND,
    BLEND_MODE_SCREEN,
    sortKey,
    GLOW_GROUP_ID,
  );

  const gravity: f32 = <f32>160.0 + <f32>rangedFromSeed(seed, 91, 0, 120);
  for (let index: u32 = 0; index < ITEM_COUNT; index += 1) {
    const angleBase: f32 = (<f32>index / <f32>ITEM_COUNT) * TAU;
    const angleJitter: f32 = <f32>signedFromSeed(seed, index * 13 + 3, 22) * <f32>0.017453292;
    const angle: f32 = angleBase + angleJitter;
    const speed: f32 = (<f32>110.0 + <f32>rangedFromSeed(seed, index * 17 + 7, 0, 140)) * energyScale;
    const radius: f32 = <f32>2.8 + <f32>rangedFromSeed(seed, index * 19 + 11, 0, 5);
    const alpha: f32 = <f32>0.48 + <f32>rangedFromSeed(seed, index * 23 + 15, 0, 42) / <f32>100.0;
    const jitterX: f32 = <f32>signedFromSeed(seed, index * 29 + 19, 5);
    const jitterY: f32 = <f32>signedFromSeed(seed, index * 31 + 21, 5);
    const vx: f32 = <f32>Math.cos(angle) * speed;
    const vy: f32 = <f32>Math.sin(angle) * speed - <f32>(energyScale * 46.0);
    writeSpawnGlowBatchItem(
      outputPtr,
      index,
      x + jitterX,
      y + jitterY,
      radius,
      alpha,
      palette(index + seed, button),
      vx,
      vy,
      <f32>0.0,
      gravity,
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

  const x: f32 = <f32>readEventX(inputPtr);
  const y: f32 = <f32>readEventY(inputPtr);
  const button = readEventButton(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const commandCap = minU32(outputCap / COMMAND_BYTES, MAX_COMMANDS);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  let written: u32 = 0;
  emitBurst(offset, x, y, seed ^ 0x9E3779B9, button, 0, 520, <f32>1.0, 12);
  written += 1;
  offset += <usize>COMMAND_BYTES;

  if (written >= commandCap) {
    return written * COMMAND_BYTES;
  }

  emitBurst(offset, x, y - <f32>4.0, seed ^ 0x7F4A7C15, button, 44, 360, <f32>0.72, 6);
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
