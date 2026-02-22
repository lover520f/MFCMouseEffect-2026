import {
  API_VERSION,
  EVENT_KIND_SCROLL,
  SPAWN_IMAGE_COMMAND_BYTES,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleEvent,
  readEventDelta,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnImage,
  writeSpawnText,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

const MAX_PARTICLES: u32 = 24;
const MIN_PARTICLES: u32 = 10;

function absI32(v: i32): i32 {
  return v < 0 ? -v : v;
}

function clampU32(v: u32, minValue: u32, maxValue: u32): u32 {
  if (v < minValue) return minValue;
  if (v > maxValue) return maxValue;
  return v;
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {}

export function mfx_plugin_on_event(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleEvent(inputLen, outputCap, SPAWN_IMAGE_COMMAND_BYTES)) {
    return 0;
  }

  const kind = readEventKind(inputPtr);
  if (kind != EVENT_KIND_SCROLL) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const delta = readEventDelta(inputPtr);
  const direction: f32 = delta >= 0 ? <f32>-1.0 : <f32>1.0;
  const eventSeed = seedFromTickMs(readEventTickMs(inputPtr));

  const scaled = <u32>(absI32(delta) / 10);
  const particleCount = clampU32(MIN_PARTICLES + scaled, MIN_PARTICLES, MAX_PARTICLES);

  let offset: usize = outputPtr;
  let written: u32 = 0;
  for (let i: u32 = 0; i < particleCount; i++) {
    const seed = eventSeed ^ (i * 0x45D9F3B);
    const remain = outputCap - written;
    const emitRing = (i % 6) == 0;

    if (emitRing && remain >= SPAWN_TEXT_COMMAND_BYTES) {
      writeSpawnText(
        offset,
        x + <f32>signedFromSeed(seed, 2, 12),
        y + <f32>signedFromSeed(seed, 4, 10),
        <f32>signedFromSeed(seed, 7, 38),
        direction * (<f32>95.0 + <f32>rangedFromSeed(seed, 11, 0, 55)),
        0.0,
        -direction * <f32>120.0,
        0.78 + (<f32>rangedFromSeed(seed, 13, 0, 22) / 100.0),
        0.0,
        0.92,
        colorFromSeed(seed ^ 0xA7F13B5D),
        rangedFromSeed(seed, 15, 0, 36),
        420 + rangedFromSeed(seed, 18, 0, 240),
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
      0.64 + (<f32>rangedFromSeed(seed, 10, 0, 68) / 100.0),
      <f32>signedFromSeed(seed, 12, 10) * 0.015,
      0.88,
      colorFromSeed(seed ^ 0x58CD246F),
      rangedFromSeed(seed, 14, 0, 28),
      420 + rangedFromSeed(seed, 16, 0, 340),
      (seed >> 3) % 4,
    );
    offset += <usize>SPAWN_IMAGE_COMMAND_BYTES;
    written += SPAWN_IMAGE_COMMAND_BYTES;
  }

  return written;
}
