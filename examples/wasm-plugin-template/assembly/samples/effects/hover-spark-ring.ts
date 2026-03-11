import {
  API_VERSION,
  EVENT_KIND_HOVER_END,
  EVENT_KIND_HOVER_START,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleEvent,
  canHandleFrameInput,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  readFrameCursorX,
  readFrameCursorY,
  readFrameDeltaMs,
  readFramePointerValid,
  readFrameTickMs,
  writeSpawnText,
} from "../../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const FRAME_SPARK_COUNT: u32 = 3;
const ENERGY_DECAY_PER_FRAME: f32 = 0.9;
const ENERGY_MIN_TO_EMIT: f32 = 0.08;

let gHovering: bool = false;
let gAnchorX: f32 = 0.0;
let gAnchorY: f32 = 0.0;
let gEnergy: f32 = 0.0;
let gPhase: f32 = 0.0;
let gSeed: u32 = 0x517CC1B7;

function clampF32(v: f32, minValue: f32, maxValue: f32): f32 {
  if (v < minValue) return minValue;
  if (v > maxValue) return maxValue;
  return v;
}

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function frameScale(deltaMs: u32): f32 {
  if (deltaMs == 0) {
    return 1.0;
  }
  return clampF32(<f32>deltaMs / 16.0, 0.5, 3.0);
}

function emitInputSpark(
  outputPtr: usize,
  outputCap: u32,
  x: f32,
  y: f32,
  seed: u32,
): u32 {
  const commandCap = minU32(outputCap / SPAWN_TEXT_COMMAND_BYTES, 2);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  for (let i: u32 = 0; i < commandCap; i += 1) {
    const commandSeed = seed ^ ((i + 1) * 0x27D4EB2D);
    writeSpawnText(
      offset,
      x + <f32>signedFromSeed(commandSeed, 2, 9),
      y + <f32>signedFromSeed(commandSeed, 5, 9),
      <f32>signedFromSeed(commandSeed, 8, 32),
      -60.0 - <f32>rangedFromSeed(commandSeed, 11, 0, 38),
      0.0,
      126.0,
      0.72,
      0.0,
      0.9,
      colorFromSeed(commandSeed),
      <u32>rangedFromSeed(commandSeed, 15, 0, 12),
      260 + <u32>rangedFromSeed(commandSeed, 18, 0, 140),
      commandSeed % 8,
    );
    offset += <usize>SPAWN_TEXT_COMMAND_BYTES;
  }

  return commandCap * SPAWN_TEXT_COMMAND_BYTES;
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {
  gHovering = false;
  gAnchorX = 0.0;
  gAnchorY = 0.0;
  gEnergy = 0.0;
  gPhase = 0.0;
  gSeed = 0x517CC1B7;
}

export function mfx_plugin_on_input(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleEvent(inputLen, outputCap, SPAWN_TEXT_COMMAND_BYTES)) {
    return 0;
  }

  const kind = readEventKind(inputPtr);
  if (kind != EVENT_KIND_HOVER_START && kind != EVENT_KIND_HOVER_END) {
    return 0;
  }

  gAnchorX = <f32>readEventX(inputPtr);
  gAnchorY = <f32>readEventY(inputPtr);
  gSeed = seedFromTickMs(readEventTickMs(inputPtr));

  if (kind == EVENT_KIND_HOVER_END) {
    gHovering = false;
    gEnergy = 0.0;
    return emitInputSpark(outputPtr, outputCap, gAnchorX, gAnchorY, gSeed ^ 0x94D049BB);
  }

  gHovering = true;
  gEnergy = 1.0;
  return emitInputSpark(outputPtr, outputCap, gAnchorX, gAnchorY, gSeed);
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleFrameInput(inputLen, outputCap, SPAWN_TEXT_COMMAND_BYTES)) {
    return 0;
  }
  if (!gHovering) {
    return 0;
  }

  if (readFramePointerValid(inputPtr)) {
    gAnchorX = <f32>readFrameCursorX(inputPtr);
    gAnchorY = <f32>readFrameCursorY(inputPtr);
  }

  const tickSeed = seedFromTickMs(readFrameTickMs(inputPtr)) ^ gSeed;
  const deltaScale = frameScale(readFrameDeltaMs(inputPtr));
  gPhase += 0.12 * deltaScale;
  gEnergy *= clampF32(ENERGY_DECAY_PER_FRAME - (deltaScale - 1.0) * 0.03, 0.74, 0.96);
  if (gEnergy < ENERGY_MIN_TO_EMIT) {
    gEnergy = 0.22;
  }

  const commandCap = minU32(outputCap / SPAWN_TEXT_COMMAND_BYTES, FRAME_SPARK_COUNT);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  for (let i: u32 = 0; i < commandCap; i += 1) {
    const unit = (<f32>i) / (<f32>FRAME_SPARK_COUNT);
    const angle = gPhase + unit * 6.2831853;
    const radius = 14.0 + <f32>rangedFromSeed(tickSeed, i + 1, 0, 18);
    const px: f32 = <f32>(gAnchorX + <f32>Math.cos(angle) * radius);
    const py: f32 = <f32>(gAnchorY + <f32>Math.sin(angle) * radius);
    const commandSeed = tickSeed ^ ((i + 1) * 0x9E3779B9);

    writeSpawnText(
      offset,
      px,
      py,
      <f32>signedFromSeed(commandSeed, 3, 24),
      -56.0 - <f32>rangedFromSeed(commandSeed, 7, 0, 42),
      0.0,
      118.0,
      0.64 + (<f32>rangedFromSeed(commandSeed, 10, 0, 24) / 100.0),
      0.0,
      0.84,
      colorFromSeed(commandSeed),
      0,
      220 + <u32>rangedFromSeed(commandSeed, 13, 0, 170),
      commandSeed % 8,
    );
    offset += <usize>SPAWN_TEXT_COMMAND_BYTES;
  }

  return commandCap * SPAWN_TEXT_COMMAND_BYTES;
}
