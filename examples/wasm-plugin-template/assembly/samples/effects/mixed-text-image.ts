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
} from "../../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const MIXED_OUTPUT_BYTES: u32 = SPAWN_TEXT_COMMAND_BYTES + SPAWN_IMAGE_COMMAND_BYTES;

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
  if (!canHandleClickEvent(inputPtr, inputLen, outputCap, MIXED_OUTPUT_BYTES)) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));

  writeSpawnText(
    outputPtr,
    x - 42.0,
    y + 18.0,
    -18.0 + <f32>signedFromSeed(seed, 4, 10),
    -98.0 - <f32>rangedFromSeed(seed, 1, 0, 20),
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
    x + 78.0,
    y - 26.0,
    54.0 + <f32>signedFromSeed(seed, 11, 18),
    -142.0 - <f32>rangedFromSeed(seed, 6, 0, 22),
    0.0,
    95.0,
    1.02 + (<f32>rangedFromSeed(seed, 14, 0, 14) / 100.0),
    0.0,
    0.95,
    colorFromSeed(seed ^ 0x0EDC7431),
    0,
    620,
    (seed >> 3) % 4,
  );

  return MIXED_OUTPUT_BYTES;
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  return 0;
}
