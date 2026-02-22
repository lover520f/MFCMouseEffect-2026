import {
  API_VERSION,
  SPAWN_IMAGE_COMMAND_BYTES,
  canHandleClickEvent,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnImage,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

const COMMAND_COUNT: u32 = 3;
const OUTPUT_BYTES: u32 = SPAWN_IMAGE_COMMAND_BYTES * COMMAND_COUNT;

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
  if (!canHandleClickEvent(inputPtr, inputLen, outputCap, OUTPUT_BYTES)) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));

  for (let i: u32 = 0; i < COMMAND_COUNT; i += 1) {
    const offset = outputPtr + <usize>(i * SPAWN_IMAGE_COMMAND_BYTES);
    const side: f32 = i == 0 ? <f32>-1.0 : i == 1 ? <f32>1.0 : <f32>0.0;
    writeSpawnImage(
      offset,
      x,
      y,
      side * (<f32>88.0 + <f32>rangedFromSeed(seed, 2 + i, 0, 42)),
      -170.0 - <f32>rangedFromSeed(seed, 8 + i, 0, 52),
      0.0,
      120.0 + <f32>i * 20.0,
      0.9 + (<f32>rangedFromSeed(seed, 11 + i, 0, 45) / 100.0),
      <f32>signedFromSeed(seed, 16 + i, 20) * 0.01,
      0.92,
      colorFromSeed(seed ^ (0xC2B2AE35 * (i + 1))),
      i * 16,
      640 + i * 90,
      i,
    );
  }

  return OUTPUT_BYTES;
}
