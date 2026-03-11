import {
  API_VERSION,
  BLEND_MODE_ADD,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  SPAWN_SPRITE_BATCH_FLAG_SCREEN_BLEND,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnSpriteBatchCommandBytesWithSemanticsAndClip,
  writeSpawnSpriteBatchHeaderWithSemanticsAndClip,
  writeSpawnSpriteBatchItem,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const ITEM_COUNT: u32 = 20;
const COMMAND_BYTES: u32 = spawnSpriteBatchCommandBytesWithSemanticsAndClip(ITEM_COUNT);
const MAX_COMMANDS: u32 = 2;
const TAU: f32 = <f32>6.283185307179586;
const SPRITE_GROUP_ID: u32 = 1002;

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function tintFor(button: u8, index: u32): u32 {
  const normalized = index % 6;
  if (button == BUTTON_RIGHT) {
    if (normalized == 0) return 0xFFFFC107;
    if (normalized == 1) return 0xFFFF7043;
    if (normalized == 2) return 0xFFFF5252;
    if (normalized == 3) return 0xFFFFF176;
    if (normalized == 4) return 0xFFFF8A65;
    return 0xFFFFD54F;
  }
  if (button == BUTTON_MIDDLE) {
    if (normalized == 0) return 0xFF7C4DFF;
    if (normalized == 1) return 0xFF40C4FF;
    if (normalized == 2) return 0xFF18FFFF;
    if (normalized == 3) return 0xFF69F0AE;
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
  const clipWidthPx: f32 = 292.0 + energyScale * 68.0;
  const clipHeightPx: f32 = 304.0 + energyScale * 74.0;
  writeSpawnSpriteBatchHeaderWithSemanticsAndClip(
    outputPtr,
    ITEM_COUNT,
    delayMs,
    lifeMs,
    SPAWN_SPRITE_BATCH_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    sortKey,
    SPRITE_GROUP_ID,
    x - clipWidthPx * 0.5,
    y - clipHeightPx * 0.5,
    clipWidthPx,
    clipHeightPx,
  );

  for (let index: u32 = 0; index < ITEM_COUNT; index += 1) {
    const angleBase: f32 = (<f32>index / <f32>ITEM_COUNT) * TAU;
    const angleJitter: f32 = <f32>signedFromSeed(seed, index * 17 + 3, 24) * <f32>0.017453292;
    const angle: f32 = angleBase + angleJitter;
    const speed: f32 = (<f32>120.0 + <f32>rangedFromSeed(seed, index * 19 + 7, 0, 170)) * energyScale;
    const spawnRadius: f32 = (<f32>24.0 + <f32>rangedFromSeed(seed, index * 13 + 1, 0, 52)) * energyScale;
    const scale: f32 = <f32>0.32 + <f32>rangedFromSeed(seed, index * 23 + 9, 0, 36) / <f32>100.0;
    const alpha: f32 = <f32>0.58 + <f32>rangedFromSeed(seed, index * 29 + 11, 0, 38) / <f32>100.0;
    const rotation: f32 = <f32>signedFromSeed(seed, index * 31 + 13, 160) * <f32>0.017453292;
    const jitterX: f32 = <f32>signedFromSeed(seed, index * 37 + 17, 5);
    const jitterY: f32 = <f32>signedFromSeed(seed, index * 41 + 19, 5);
    const vx: f32 = <f32>Math.cos(angle) * speed;
    const vy: f32 = <f32>Math.sin(angle) * speed - <f32>(energyScale * 42.0);
    const ax: f32 = <f32>signedFromSeed(seed, index * 43 + 23, 26);
    const ay: f32 = <f32>150.0 + <f32>rangedFromSeed(seed, index * 47 + 29, 0, 120);
    const imageId: u32 = rangedFromSeed(seed, index * 53 + 31, 0, 2);
    writeSpawnSpriteBatchItem(
      outputPtr,
      index,
      x + <f32>Math.cos(angle) * spawnRadius + jitterX,
      y + <f32>Math.sin(angle) * spawnRadius + jitterY,
      scale,
      rotation,
      alpha,
      tintFor(button, index + seed),
      imageId,
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
  emitBurst(offset, x, y, seed ^ 0xA511E9B3, button, 0, 580, <f32>1.0, 18);
  written += 1;
  offset += <usize>COMMAND_BYTES;

  if (written >= commandCap) {
    return written * COMMAND_BYTES;
  }

  emitBurst(offset, x, y - <f32>5.0, seed ^ 0x5F72C913, button, 48, 380, <f32>0.68, 10);
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
