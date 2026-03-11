import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  PULSE_KIND_RIPPLE,
  PULSE_KIND_STAR,
  SPAWN_PULSE_COMMAND_BYTES,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnPulse,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const MAX_COMMANDS: u32 = 4;

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
}

function paletteFill(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0x52FF9A3D;
  if (button == BUTTON_MIDDLE) return 0x5E8C6BFF;
  return 0x5449D8FF;
}

function paletteStroke(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFF7A00;
  if (button == BUTTON_MIDDLE) return 0xFF7B5CFF;
  return 0xFF12D8FF;
}

function paletteGlow(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0x66FF7A00;
  if (button == BUTTON_MIDDLE) return 0x667B5CFF;
  return 0x6612D8FF;
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
  if (!canHandleEvent(inputLen, outputCap, SPAWN_PULSE_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const button = readEventButton(inputPtr);
  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const fillArgb = paletteFill(button);
  const strokeArgb = paletteStroke(button);
  const glowArgb = paletteGlow(button);
  const commandCap = minU32(outputCap / SPAWN_PULSE_COMMAND_BYTES, MAX_COMMANDS);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  let written: u32 = 0;

  writeSpawnPulse(
    offset,
    x,
    y,
    6.0,
    56.0 + <f32>rangedFromSeed(seed, 2, 0, 12),
    2.2,
    0.96,
    fillArgb,
    strokeArgb,
    glowArgb,
    0,
    340,
    PULSE_KIND_RIPPLE,
  );
  offset += <usize>SPAWN_PULSE_COMMAND_BYTES;
  written += 1;

  if (written >= commandCap) {
    return written * SPAWN_PULSE_COMMAND_BYTES;
  }

  writeSpawnPulse(
    offset,
    x,
    y,
    12.0,
    34.0 + <f32>rangedFromSeed(seed, 12, 0, 10),
    1.6,
    0.82,
    fillArgb,
    strokeArgb,
    glowArgb,
    28,
    260,
    PULSE_KIND_STAR,
  );
  offset += <usize>SPAWN_PULSE_COMMAND_BYTES;
  written += 1;

  if (written >= commandCap) {
    return written * SPAWN_PULSE_COMMAND_BYTES;
  }

  writeSpawnPulse(
    offset,
    x,
    y,
    18.0,
    86.0 + <f32>rangedFromSeed(seed, 15, 0, 18),
    3.2,
    0.56,
    0x18FFFFFF,
    strokeArgb,
    glowArgb,
    64,
    460,
    PULSE_KIND_RIPPLE,
  );
  offset += <usize>SPAWN_PULSE_COMMAND_BYTES;
  written += 1;

  if (written >= commandCap) {
    return written * SPAWN_PULSE_COMMAND_BYTES;
  }

  const sideShift: f32 = button == BUTTON_RIGHT ? -18.0 : 18.0;
  writeSpawnPulse(
    offset,
    x,
    y,
    8.0,
    28.0 + <f32>rangedFromSeed(seed, 21, 0, 10),
    1.4,
    0.68,
    0x22FFFFFF,
    strokeArgb,
    glowArgb,
    96,
    220,
    PULSE_KIND_STAR,
  );
  written += 1;

  return written * SPAWN_PULSE_COMMAND_BYTES;
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
