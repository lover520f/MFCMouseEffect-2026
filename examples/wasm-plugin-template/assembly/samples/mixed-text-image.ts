import {
  API_VERSION,
  SPAWN_IMAGE_COMMAND_BYTES,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleClickEvent,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnImage,
  writeSpawnText,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

const MIXED_OUTPUT_BYTES: u32 = SPAWN_TEXT_COMMAND_BYTES + SPAWN_IMAGE_COMMAND_BYTES;

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
  if (!canHandleClickEvent(inputPtr, inputLen, outputCap, MIXED_OUTPUT_BYTES)) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));

  writeSpawnText(
    outputPtr,
    x,
    y,
    <f32>signedFromSeed(seed, 4, 12),
    -86.0 - <f32>rangedFromSeed(seed, 1, 0, 26),
    0.0,
    160.0,
    1.0,
    0.0,
    1.0,
    colorFromSeed(seed ^ 0x99AF1203),
    0,
    620,
    seed % 8,
  );

  writeSpawnImage(
    outputPtr + <usize>SPAWN_TEXT_COMMAND_BYTES,
    x,
    y,
    <f32>signedFromSeed(seed, 11, 70),
    -155.0 - <f32>rangedFromSeed(seed, 6, 0, 46),
    0.0,
    95.0,
    0.94 + (<f32>rangedFromSeed(seed, 14, 0, 30) / 100.0),
    0.0,
    0.95,
    colorFromSeed(seed ^ 0x0EDC7431),
    24,
    560,
    (seed >> 3) % 4,
  );

  return MIXED_OUTPUT_BYTES;
}
