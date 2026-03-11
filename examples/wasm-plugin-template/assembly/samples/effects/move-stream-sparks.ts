import {
  API_VERSION,
  EVENT_KIND_MOVE,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleEvent,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnText,
} from "../../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const MAX_COMMANDS_PER_EVENT: u32 = 3;

function minU32(a: u32, b: u32): u32 {
  return a < b ? a : b;
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
  if (!canHandleEvent(inputLen, outputCap, SPAWN_TEXT_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_MOVE) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const commandCap = minU32(outputCap / SPAWN_TEXT_COMMAND_BYTES, MAX_COMMANDS_PER_EVENT);
  if (commandCap == 0) {
    return 0;
  }

  let offset = outputPtr;
  for (let i: u32 = 0; i < commandCap; i += 1) {
    const commandSeed = seed ^ ((i + 1) * 0x9E3779B9);
    writeSpawnText(
      offset,
      x + <f32>signedFromSeed(commandSeed, 2, 8),
      y + <f32>signedFromSeed(commandSeed, 5, 8),
      <f32>signedFromSeed(commandSeed, 8, 44),
      -90.0 - <f32>rangedFromSeed(commandSeed, 12, 0, 58),
      0.0,
      185.0,
      0.78 + (<f32>rangedFromSeed(commandSeed, 15, 0, 14) / 100.0),
      0.0,
      0.9,
      colorFromSeed(commandSeed),
      <u32>rangedFromSeed(commandSeed, 18, 0, 20),
      340 + <u32>rangedFromSeed(commandSeed, 20, 0, 220),
      commandSeed % 8,
    );
    offset += <usize>SPAWN_TEXT_COMMAND_BYTES;
  }

  return commandCap * SPAWN_TEXT_COMMAND_BYTES;
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  return 0;
}

