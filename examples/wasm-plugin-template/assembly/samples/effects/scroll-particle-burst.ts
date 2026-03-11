import {
  API_VERSION,
  EVENT_KIND_SCROLL,
  SPAWN_IMAGE_COMMAND_BYTES,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleEvent,
  canHandleFrameInput,
  readEventDelta,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  readFrameCursorX,
  readFrameCursorY,
  readFrameDeltaMs,
  readFramePointerValid,
  readFrameTickMs,
  writeSpawnImage,
  writeSpawnText,
} from "../../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const INPUT_MIN_PARTICLES: u32 = 4;
const INPUT_MAX_PARTICLES: u32 = 12;
const FRAME_MIN_PARTICLES: u32 = 1;
const FRAME_MAX_PARTICLES: u32 = 16;

const ENERGY_GAIN_SCALE: f32 = 0.15;
const ENERGY_CAP: f32 = 24.0;
const ENERGY_DECAY_PER_FRAME: f32 = 0.87;
const ENERGY_IDLE_DECAY_PER_FRAME: f32 = 0.74;
const FOLLOW_CURSOR_WINDOW_MS: u64 = 180;
const IDLE_SOFT_STOP_MS: u64 = 1200;

let gEmitterX: f32 = 0.0;
let gEmitterY: f32 = 0.0;
let gDirection: f32 = -1.0;
let gEnergy: f32 = 0.0;
let gLastInputTickMs: u64 = 0;
let gSeed: u32 = 0x3C6EF372;
let gHasEmitter: bool = false;

function absI32(v: i32): i32 {
  return v < 0 ? -v : v;
}

function clampU32(v: u32, minValue: u32, maxValue: u32): u32 {
  if (v < minValue) return minValue;
  if (v > maxValue) return maxValue;
  return v;
}

function clampF32(v: f32, minValue: f32, maxValue: f32): f32 {
  if (v < minValue) return minValue;
  if (v > maxValue) return maxValue;
  return v;
}

function nextSeed(seed: u32): u32 {
  return seed * 1664525 + 1013904223;
}

function normalizeFrameScale(deltaMs: u32): f32 {
  if (deltaMs == 0) {
    return 1.0;
  }
  return clampF32(<f32>deltaMs / 16.0, 0.5, 3.5);
}

function emitMixedBurst(
  outputPtr: usize,
  outputCap: u32,
  x: f32,
  y: f32,
  direction: f32,
  baseSeed: u32,
  particleCount: u32,
  textChancePercent: u32,
): u32 {
  let offset: usize = outputPtr;
  let written: u32 = 0;

  for (let i: u32 = 0; i < particleCount; i++) {
    const seed = baseSeed ^ (i * 0x45D9F3B);
    const remain = outputCap - written;
    const emitText = (<u32>rangedFromSeed(seed, 2, 0, 99) < textChancePercent);

    if (emitText && remain >= SPAWN_TEXT_COMMAND_BYTES) {
      writeSpawnText(
        offset,
        x + <f32>signedFromSeed(seed, 2, 12),
        y + <f32>signedFromSeed(seed, 4, 10),
        <f32>signedFromSeed(seed, 7, 38),
        direction * (<f32>95.0 + <f32>rangedFromSeed(seed, 11, 0, 55)),
        0.0,
        -direction * <f32>120.0,
        0.76 + (<f32>rangedFromSeed(seed, 13, 0, 24) / 100.0),
        0.0,
        0.92,
        colorFromSeed(seed ^ 0xA7F13B5D),
        rangedFromSeed(seed, 15, 0, 36),
        380 + rangedFromSeed(seed, 18, 0, 260),
        seed % 8,
      );
      offset += <usize>SPAWN_TEXT_COMMAND_BYTES;
      written += SPAWN_TEXT_COMMAND_BYTES;
      continue;
    }

    if (remain < SPAWN_IMAGE_COMMAND_BYTES) {
      break;
    }

    const vx = <f32>signedFromSeed(seed, 6, 260);
    const vy: f32 = direction * (<f32>120.0 + <f32>rangedFromSeed(seed, 8, 0, 210));
    writeSpawnImage(
      offset,
      x + <f32>signedFromSeed(seed, 3, 8),
      y + <f32>signedFromSeed(seed, 5, 8),
      vx,
      vy,
      0.0,
      -direction * (<f32>160.0 + <f32>rangedFromSeed(seed, 9, 0, 120)),
      0.56 + (<f32>rangedFromSeed(seed, 10, 0, 76) / 100.0),
      <f32>signedFromSeed(seed, 12, 10) * 0.015,
      0.88,
      colorFromSeed(seed ^ 0x58CD246F),
      rangedFromSeed(seed, 14, 0, 28),
      360 + rangedFromSeed(seed, 16, 0, 360),
      (seed >> 3) % 4,
    );
    offset += <usize>SPAWN_IMAGE_COMMAND_BYTES;
    written += SPAWN_IMAGE_COMMAND_BYTES;
  }

  return written;
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {
  gEmitterX = 0.0;
  gEmitterY = 0.0;
  gDirection = -1.0;
  gEnergy = 0.0;
  gLastInputTickMs = 0;
  gSeed = 0x3C6EF372;
  gHasEmitter = false;
}

export function mfx_plugin_on_input(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleEvent(inputLen, outputCap, SPAWN_IMAGE_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_SCROLL) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const delta = readEventDelta(inputPtr);
  const absDelta = absI32(delta);
  const eventTick = readEventTickMs(inputPtr);

  gEmitterX = x;
  gEmitterY = y;
  gDirection = delta >= 0 ? -1.0 : 1.0;
  gLastInputTickMs = eventTick;
  gHasEmitter = true;

  const gain = clampF32(<f32>absDelta * ENERGY_GAIN_SCALE, 1.2, 8.0);
  gEnergy = clampF32(gEnergy + gain, 0.0, ENERGY_CAP);
  gSeed = seedFromTickMs(eventTick) ^ (<u32>absDelta * 0x9E3779B9);

  const burstCount = clampU32(
    INPUT_MIN_PARTICLES + <u32>(absDelta / 120),
    INPUT_MIN_PARTICLES,
    INPUT_MAX_PARTICLES,
  );
  return emitMixedBurst(
    outputPtr,
    outputCap,
    gEmitterX,
    gEmitterY,
    gDirection,
    gSeed,
    burstCount,
    24,
  );
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleFrameInput(inputLen, outputCap, SPAWN_IMAGE_COMMAND_BYTES)) {
    return 0;
  }
  if (!gHasEmitter) {
    return 0;
  }

  const nowTick = readFrameTickMs(inputPtr);
  const frameScale = normalizeFrameScale(readFrameDeltaMs(inputPtr));

  if (
    readFramePointerValid(inputPtr) &&
    gLastInputTickMs > 0 &&
    nowTick >= gLastInputTickMs &&
    (nowTick - gLastInputTickMs) <= FOLLOW_CURSOR_WINDOW_MS
  ) {
    gEmitterX = <f32>readFrameCursorX(inputPtr);
    gEmitterY = <f32>readFrameCursorY(inputPtr);
  }

  const idleMs = nowTick >= gLastInputTickMs ? (nowTick - gLastInputTickMs) : 0;
  const decay = idleMs > 280 ? ENERGY_IDLE_DECAY_PER_FRAME : ENERGY_DECAY_PER_FRAME;
  const decayScale = clampF32(decay - (frameScale - 1.0) * 0.06, 0.42, 0.96);
  gEnergy *= decayScale;

  if (idleMs > IDLE_SOFT_STOP_MS && gEnergy < 0.14) {
    gHasEmitter = false;
    gEnergy = 0.0;
    return 0;
  }
  if (gEnergy < 0.05) {
    return 0;
  }

  gSeed = nextSeed(gSeed ^ <u32>nowTick);
  const frameCount = clampU32(
    FRAME_MIN_PARTICLES + <u32>gEnergy,
    FRAME_MIN_PARTICLES,
    FRAME_MAX_PARTICLES,
  );

  const frameBytes = emitMixedBurst(
    outputPtr,
    outputCap,
    gEmitterX,
    gEmitterY,
    gDirection,
    gSeed,
    frameCount,
    18,
  );

  if (frameBytes > 0) {
    const emittedCommands = frameBytes / SPAWN_IMAGE_COMMAND_BYTES;
    gEnergy = clampF32(gEnergy - <f32>emittedCommands * 0.14, 0.0, ENERGY_CAP);
  }
  return frameBytes;
}
