import {
  API_VERSION,
  EVENT_KIND_SCROLL,
  SPAWN_IMAGE_AFFINE_COMMAND_BYTES,
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
  writeSpawnImageAffine,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const PALETTE_SIZE: u32 = 10;
const MAX_COMMANDS_SOFT: u32 = 72;

const INPUT_MIN_PARTICLES: u32 = 3;
const INPUT_MAX_PARTICLES: u32 = 8;
const FRAME_MIN_PARTICLES: u32 = 1;
const FRAME_MAX_PARTICLES: u32 = 12;

const MIN_TRAIL_SEGMENTS: u32 = 1;
const MAX_TRAIL_SEGMENTS: u32 = 2;

const ENERGY_DECAY_PER_FRAME: f32 = 0.86;
const ENERGY_IDLE_DECAY_PER_FRAME: f32 = 0.72;
const ENERGY_GAIN_SCALE: f32 = 0.18;
const ENERGY_CAP: f32 = 20.0;
const FOLLOW_CURSOR_WINDOW_MS: u64 = 180;
const IDLE_SOFT_STOP_MS: u64 = 1400;

let gEmitterX: f32 = 0.0;
let gEmitterY: f32 = 0.0;
let gDirectionSign: f32 = 1.0;
let gEnergy: f32 = 0.0;
let gLastInputTickMs: u64 = 0;
let gFrameSeed: u32 = 0x51F15EED;
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

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function maxF32(a: f32, b: f32): f32 {
  return a > b ? a : b;
}

function maxU32(a: u32, b: u32): u32 {
  return a > b ? a : b;
}

function nextSeed(seed: u32): u32 {
  return seed * 1664525 + 1013904223;
}

function paletteColor(index: u32): u32 {
  switch (index % PALETTE_SIZE) {
    case 0: return 0xFFFF0080;
    case 1: return 0xFFFF0088;
    case 2: return 0xFFFF00FF;
    case 3: return 0xFF8800FF;
    case 4: return 0xFF0080FF;
    case 5: return 0xFF00FFFF;
    case 6: return 0xFF00FF80;
    case 7: return 0xFF80FF00;
    case 8: return 0xFFFFFF00;
    default: return 0xFFFF8000;
  }
}

function emitParticle(
  outPtr: usize,
  x: f32,
  y: f32,
  vx: f32,
  vy: f32,
  ay: f32,
  scale: f32,
  alpha: f32,
  tintArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  seed: u32,
): void {
  const angleJitter = <f32>signedFromSeed(seed, 20, 14) * 0.01;
  const stretchX = 1.0 + (<f32>signedFromSeed(seed, 22, 8) / 100.0);
  const stretchY = 1.0 + (<f32>signedFromSeed(seed, 24, 8) / 100.0);
  const cosA = <f32>Math.cos(angleJitter);
  const sinA = <f32>Math.sin(angleJitter);
  const affineM11: f32 = <f32>(stretchX * cosA);
  const affineM12: f32 = <f32>(-stretchY * sinA);
  const affineM21: f32 = <f32>(stretchX * sinA);
  const affineM22: f32 = <f32>(stretchY * cosA);

  writeSpawnImageAffine(
    outPtr,
    x + <f32>signedFromSeed(seed, 5, 9),
    y + <f32>signedFromSeed(seed, 8, 9),
    vx,
    vy,
    0.0,
    ay,
    scale,
    0.0,
    alpha,
    tintArgb,
    delayMs,
    lifeMs,
    0,
    affineM11,
    affineM12,
    affineM21,
    affineM22,
    <f32>signedFromSeed(seed, 27, 4),
    <f32>signedFromSeed(seed, 29, 4),
    0,
    true,
  );
}

function emitPulseAccent(
  outPtr: usize,
  x: f32,
  y: f32,
  directionSign: f32,
  seed: u32,
): void {
  const scale: f32 = 0.10 + (<f32>rangedFromSeed(seed, 2, 0, 16) / 100.0);
  const alpha: f32 = 0.30 + (<f32>rangedFromSeed(seed, 4, 0, 16) / 100.0);
  const lifeMs: u32 = 360 + <u32>rangedFromSeed(seed, 6, 0, 120);
  const delayMs: u32 = <u32>rangedFromSeed(seed, 8, 0, 28);
  const vx: f32 = <f32>signedFromSeed(seed, 10, 90);
  const vy: f32 = directionSign * (110.0 + <f32>rangedFromSeed(seed, 13, 0, 120));
  const ay: f32 = directionSign * (95.0 + <f32>rangedFromSeed(seed, 16, 0, 95));

  emitParticle(
    outPtr,
    x,
    y,
    vx,
    vy,
    ay,
    scale,
    alpha,
    paletteColor(seed + 3),
    delayMs,
    lifeMs,
    seed ^ 0x9E3779B9,
  );
}

function emitBurst(
  outputPtr: usize,
  outputCap: u32,
  x: f32,
  y: f32,
  directionSign: f32,
  baseSeed: u32,
  baseParticles: u32,
  trailSegments: u32,
  accentChancePercent: u32,
): u32 {
  const commandCapByBytes = outputCap / SPAWN_IMAGE_AFFINE_COMMAND_BYTES;
  const commandCap = minU32(commandCapByBytes, MAX_COMMANDS_SOFT);
  if (commandCap == 0) {
    return 0;
  }

  let offset: usize = outputPtr;
  let writtenCommands: u32 = 0;

  for (let i: u32 = 0; i < baseParticles; i += 1) {
    if (writtenCommands >= commandCap) {
      break;
    }

    const particleSeed = baseSeed ^ ((i + 1) * 0x45D9F3B);
    const angleRad: f32 = <f32>rangedFromSeed(particleSeed, 1, 0, 6283) * <f32>0.001;
    const speed: f32 = <f32>rangedFromSeed(particleSeed, 3, 220, 520);
    const vx: f32 = <f32>Math.cos(angleRad) * speed;
    let vy: f32 = <f32>Math.sin(angleRad) * speed;
    if (directionSign > 0.0) {
      vy += <f32>rangedFromSeed(particleSeed, 7, 140, 320);
    } else {
      vy -= <f32>rangedFromSeed(particleSeed, 7, 220, 420);
    }

    const ay: f32 = directionSign * 360.0;
    const sizePx: f32 = <f32>rangedFromSeed(particleSeed, 10, 3, 9);
    const scale: f32 = maxF32(0.03, sizePx / 96.0);
    const alpha: f32 = 0.78 + (<f32>rangedFromSeed(particleSeed, 12, 0, 14) / 100.0);
    const tintArgb: u32 = paletteColor(particleSeed);
    const delayMs: u32 = <u32>rangedFromSeed(particleSeed, 14, 0, 8);
    const lifeMs: u32 = 340 + <u32>rangedFromSeed(particleSeed, 16, 0, 220);

    emitParticle(
      offset,
      x,
      y,
      vx,
      vy,
      ay,
      scale,
      alpha,
      tintArgb,
      delayMs,
      lifeMs,
      particleSeed,
    );
    offset += <usize>SPAWN_IMAGE_AFFINE_COMMAND_BYTES;
    writtenCommands += 1;

    for (let tail: u32 = 1; tail <= trailSegments; tail += 1) {
      if (writtenCommands >= commandCap) {
        break;
      }
      const tailSeed = particleSeed ^ ((tail + 1) * 0x7C2B2AE3);
      const velocityFade = maxF32(0.72, 1.0 - <f32>tail * 0.14);
      const tailVx = vx * velocityFade;
      const tailVy = vy * velocityFade;
      const tailScale = maxF32(0.02, scale * (1.0 - <f32>tail * 0.24));
      const tailAlpha = maxF32(0.12, alpha * (0.40 - <f32>tail * 0.10));
      const tailDelay = delayMs + tail * (14 + <u32>rangedFromSeed(tailSeed, 19, 0, 6));
      const tailLife = maxU32(120, lifeMs > tail * 180 ? lifeMs - tail * 180 : 0);

      emitParticle(
        offset,
        x,
        y,
        tailVx,
        tailVy,
        ay,
        tailScale,
        tailAlpha,
        tintArgb,
        tailDelay,
        tailLife,
        tailSeed,
      );
      offset += <usize>SPAWN_IMAGE_AFFINE_COMMAND_BYTES;
      writtenCommands += 1;
    }
  }

  if (
    writtenCommands < commandCap &&
    <u32>rangedFromSeed(baseSeed, 9, 0, 99) < accentChancePercent
  ) {
    emitPulseAccent(offset, x, y, directionSign, baseSeed ^ 0xA7F13B5D);
    writtenCommands += 1;
  }

  return writtenCommands * SPAWN_IMAGE_AFFINE_COMMAND_BYTES;
}

function normalizeFrameDelta(deltaMs: u32): f32 {
  if (deltaMs == 0) {
    return 1.0;
  }
  return clampF32(<f32>deltaMs / 16.0, 0.5, 3.5);
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {
  gEmitterX = 0.0;
  gEmitterY = 0.0;
  gDirectionSign = 1.0;
  gEnergy = 0.0;
  gLastInputTickMs = 0;
  gFrameSeed = 0x51F15EED;
  gHasEmitter = false;
}

export function mfx_plugin_on_input(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleEvent(inputLen, outputCap, SPAWN_IMAGE_AFFINE_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_SCROLL) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const delta = readEventDelta(inputPtr);
  const absDelta = absI32(delta);

  gEmitterX = x;
  gEmitterY = y;
  gDirectionSign = delta >= 0 ? 1.0 : -1.0;
  gLastInputTickMs = readEventTickMs(inputPtr);
  gHasEmitter = true;

  const gain = clampF32(<f32>absDelta * ENERGY_GAIN_SCALE, 1.4, 8.5);
  gEnergy = clampF32(gEnergy + gain, 0.0, ENERGY_CAP);
  gFrameSeed = seedFromTickMs(gLastInputTickMs) ^ (<u32>absDelta * 0x45D9F3B);

  const instantParticles = clampU32(
    INPUT_MIN_PARTICLES + <u32>(absDelta / 180),
    INPUT_MIN_PARTICLES,
    INPUT_MAX_PARTICLES,
  );
  return emitBurst(
    outputPtr,
    outputCap,
    gEmitterX,
    gEmitterY,
    gDirectionSign,
    gFrameSeed,
    instantParticles,
    MIN_TRAIL_SEGMENTS,
    22,
  );
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleFrameInput(inputLen, outputCap, SPAWN_IMAGE_AFFINE_COMMAND_BYTES)) {
    return 0;
  }
  if (!gHasEmitter) {
    return 0;
  }

  const nowTick = readFrameTickMs(inputPtr);
  const frameScale = normalizeFrameDelta(readFrameDeltaMs(inputPtr));

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

  if (idleMs > IDLE_SOFT_STOP_MS && gEnergy < 0.16) {
    gHasEmitter = false;
    gEnergy = 0.0;
    return 0;
  }
  if (gEnergy < 0.05) {
    return 0;
  }

  gFrameSeed = nextSeed(gFrameSeed ^ <u32>nowTick);
  const frameParticles = clampU32(
    FRAME_MIN_PARTICLES + <u32>gEnergy,
    FRAME_MIN_PARTICLES,
    FRAME_MAX_PARTICLES,
  );
  const trailSegments = clampU32(
    MIN_TRAIL_SEGMENTS + <u32>(gEnergy / 7.0),
    MIN_TRAIL_SEGMENTS,
    MAX_TRAIL_SEGMENTS,
  );

  const frameBytes = emitBurst(
    outputPtr,
    outputCap,
    gEmitterX,
    gEmitterY,
    gDirectionSign,
    gFrameSeed,
    frameParticles,
    trailSegments,
    12,
  );

  if (frameBytes > 0) {
    const emittedCommands = frameBytes / SPAWN_IMAGE_AFFINE_COMMAND_BYTES;
    gEnergy = clampF32(gEnergy - <f32>emittedCommands * 0.16, 0.0, ENERGY_CAP);
  }
  return frameBytes;
}
