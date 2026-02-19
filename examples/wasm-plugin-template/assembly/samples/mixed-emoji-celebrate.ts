import {
  API_VERSION,
  SPAWN_IMAGE_COMMAND_BYTES,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleClick,
  readClickTickMs,
  readClickX,
  readClickY,
  writeSpawnImage,
  writeSpawnText,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

const TEXT_COUNT: u32 = 2;
const IMAGE_COUNT: u32 = 2;
const OUTPUT_BYTES: u32 = (SPAWN_TEXT_COMMAND_BYTES * TEXT_COUNT) + (SPAWN_IMAGE_COMMAND_BYTES * IMAGE_COUNT);

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {}

export function mfx_plugin_on_click(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleClick(inputLen, outputCap, OUTPUT_BYTES)) {
    return 0;
  }

  const x = <f32>readClickX(inputPtr);
  const y = <f32>readClickY(inputPtr);
  const seed = seedFromTickMs(readClickTickMs(inputPtr));

  for (let i: u32 = 0; i < TEXT_COUNT; i += 1) {
    const offset = outputPtr + <usize>(i * SPAWN_TEXT_COMMAND_BYTES);
    const side: f32 = i == 0 ? <f32>-1.0 : <f32>1.0;
    writeSpawnText(
      offset,
      x,
      y,
      side * (<f32>20.0 + <f32>rangedFromSeed(seed, 2 + i, 0, 16)),
      -74.0 - <f32>rangedFromSeed(seed, 7 + i, 0, 24),
      0.0,
      170.0,
      1.0 + <f32>i * 0.1,
      side * <f32>0.12,
      1.0,
      colorFromSeed(seed ^ (0x6D2B79F5 * (i + 1))),
      i * 28,
      620 + i * 90,
      (seed + i * 3) % 12,
    );
  }

  const imageBase = outputPtr + <usize>(TEXT_COUNT * SPAWN_TEXT_COMMAND_BYTES);
  for (let i: u32 = 0; i < IMAGE_COUNT; i += 1) {
    const offset = imageBase + <usize>(i * SPAWN_IMAGE_COMMAND_BYTES);
    const spread: f32 = i == 0 ? <f32>-1.0 : <f32>1.0;
    writeSpawnImage(
      offset,
      x,
      y,
      spread * (<f32>76.0 + <f32>rangedFromSeed(seed, 11 + i, 0, 34)),
      -150.0 - <f32>rangedFromSeed(seed, 16 + i, 0, 44),
      0.0,
      110.0,
      0.96 + <f32>i * 0.08,
      <f32>signedFromSeed(seed, 20 + i, 12) * 0.01,
      0.95,
      colorFromSeed(seed ^ (0x9E3779B9 * (i + 5))),
      14 + i * 34,
      540 + i * 120,
      i,
    );
  }

  return OUTPUT_BYTES;
}
