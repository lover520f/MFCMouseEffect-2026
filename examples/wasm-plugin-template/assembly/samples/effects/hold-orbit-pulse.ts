import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_HOLD_END,
  EVENT_KIND_HOLD_START,
  EVENT_KIND_HOLD_UPDATE,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleEvent,
  canHandleFrameInput,
  readEventButton,
  readEventHoldMs,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  readFrameCursorX,
  readFrameCursorY,
  readFrameDeltaMs,
  readFrameHoldActive,
  readFramePointerValid,
  readFrameTickMs,
  writeSpawnText,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const ORBIT_POINTS: u32 = 4;
const ENERGY_GAIN_PER_UPDATE: f32 = 0.14;
const ENERGY_DECAY_PER_FRAME: f32 = 0.92;
const ENERGY_MIN_TO_EMIT: f32 = 0.06;
const ENERGY_CAP: f32 = 5.0;

let gActive: bool = false;
let gAnchorX: f32 = 0.0;
let gAnchorY: f32 = 0.0;
let gButton: u8 = BUTTON_LEFT;
let gEnergy: f32 = 0.0;
let gPhase: f32 = 0.0;
let gSeed: u32 = 0xA341316C;

function clampF32(v: f32, minValue: f32, maxValue: f32): f32 {
  if (v < minValue) return minValue;
  if (v > maxValue) return maxValue;
  return v;
}

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function buttonColor(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFF7A00;
  if (button == BUTTON_MIDDLE) return 0xFF7B5CFF;
  return 0xFF2DE3FF;
}

function emitBurstAt(
  outputPtr: usize,
  outputCap: u32,
  x: f32,
  y: f32,
  color: u32,
  seed: u32,
): u32 {
  const commandCap = minU32(outputCap / SPAWN_TEXT_COMMAND_BYTES, 2);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  for (let i: u32 = 0; i < commandCap; i += 1) {
    const commandSeed = seed ^ ((i + 1) * 0x85EBCA6B);
    writeSpawnText(
      offset,
      x + <f32>signedFromSeed(commandSeed, 1, 6),
      y + <f32>signedFromSeed(commandSeed, 4, 6),
      <f32>signedFromSeed(commandSeed, 7, 34),
      -66.0 - <f32>rangedFromSeed(commandSeed, 10, 0, 44),
      0.0,
      140.0,
      0.85,
      0.0,
      0.86,
      color,
      <u32>rangedFromSeed(commandSeed, 13, 0, 16),
      280 + <u32>rangedFromSeed(commandSeed, 17, 0, 160),
      commandSeed % 8,
    );
    offset += <usize>SPAWN_TEXT_COMMAND_BYTES;
  }
  return commandCap * SPAWN_TEXT_COMMAND_BYTES;
}

function frameScale(deltaMs: u32): f32 {
  if (deltaMs == 0) {
    return 1.0;
  }
  return clampF32(<f32>deltaMs / 16.0, 0.5, 3.0);
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {
  gActive = false;
  gAnchorX = 0.0;
  gAnchorY = 0.0;
  gButton = BUTTON_LEFT;
  gEnergy = 0.0;
  gPhase = 0.0;
  gSeed = 0xA341316C;
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
  if (
    kind != EVENT_KIND_HOLD_START &&
    kind != EVENT_KIND_HOLD_UPDATE &&
    kind != EVENT_KIND_HOLD_END
  ) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const holdMs = readEventHoldMs(inputPtr);
  const tick = readEventTickMs(inputPtr);
  const seed = seedFromTickMs(tick);
  const button = readEventButton(inputPtr);

  gAnchorX = x;
  gAnchorY = y;
  gSeed = seed ^ (<u32>holdMs * 0x9E3779B9);
  if (button == BUTTON_LEFT || button == BUTTON_RIGHT || button == BUTTON_MIDDLE) {
    gButton = button;
  }

  if (kind == EVENT_KIND_HOLD_END) {
    const endColor = buttonColor(gButton);
    gActive = false;
    gEnergy = 0.0;
    return emitBurstAt(outputPtr, outputCap, gAnchorX, gAnchorY, endColor, gSeed ^ 0x7F4A7C15);
  }

  gActive = true;
  const gain = clampF32(1.0 + (<f32>holdMs * ENERGY_GAIN_PER_UPDATE / 120.0), 1.0, 2.2);
  gEnergy = clampF32(gEnergy + gain, 0.0, ENERGY_CAP);
  return emitBurstAt(outputPtr, outputCap, gAnchorX, gAnchorY, buttonColor(gButton), gSeed);
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
  if (!gActive) {
    return 0;
  }
  if (!readFrameHoldActive(inputPtr)) {
    gActive = false;
    gEnergy = 0.0;
    return 0;
  }

  if (readFramePointerValid(inputPtr)) {
    gAnchorX = <f32>readFrameCursorX(inputPtr);
    gAnchorY = <f32>readFrameCursorY(inputPtr);
  }

  const deltaScale = frameScale(readFrameDeltaMs(inputPtr));
  gPhase += 0.14 * deltaScale;
  gEnergy *= clampF32(ENERGY_DECAY_PER_FRAME - (deltaScale - 1.0) * 0.03, 0.72, 0.96);
  if (gEnergy < ENERGY_MIN_TO_EMIT) {
    return 0;
  }

  const commandCap = minU32(outputCap / SPAWN_TEXT_COMMAND_BYTES, ORBIT_POINTS);
  if (commandCap == 0) {
    return 0;
  }

  const tickSeed = seedFromTickMs(readFrameTickMs(inputPtr));
  const color = buttonColor(gButton);
  let offset = outputPtr;
  for (let i: u32 = 0; i < commandCap; i += 1) {
    const unit = (<f32>i) / (<f32>ORBIT_POINTS);
    const angle = gPhase + unit * 6.2831853;
    const orbitRadius = 18.0 + gEnergy * 8.0 + <f32>rangedFromSeed(tickSeed, i + 2, 0, 6);
    const px: f32 = <f32>(gAnchorX + <f32>Math.cos(angle) * orbitRadius);
    const py: f32 = <f32>(gAnchorY + <f32>Math.sin(angle) * orbitRadius);

    const commandSeed = tickSeed ^ ((i + 1) * 0x45D9F3B);
    writeSpawnText(
      offset,
      px,
      py,
      <f32>signedFromSeed(commandSeed, 3, 26),
      -52.0 - <f32>rangedFromSeed(commandSeed, 7, 0, 36),
      0.0,
      110.0,
      0.70 + gEnergy * 0.05,
      0.0,
      0.78,
      color,
      0,
      260 + <u32>rangedFromSeed(commandSeed, 10, 0, 170),
      commandSeed % 8,
    );
    offset += <usize>SPAWN_TEXT_COMMAND_BYTES;
  }

  return commandCap * SPAWN_TEXT_COMMAND_BYTES;
}
