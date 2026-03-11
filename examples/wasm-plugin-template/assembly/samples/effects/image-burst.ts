import {
  API_VERSION,
  SPAWN_IMAGE_COMMAND_BYTES,
  canHandleClickEvent,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnImage,
} from "../../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const COMMAND_COUNT: u32 = 3;
const OUTPUT_BYTES: u32 = SPAWN_IMAGE_COMMAND_BYTES * COMMAND_COUNT;

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
  if (!canHandleClickEvent(inputPtr, inputLen, outputCap, OUTPUT_BYTES)) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const offsetsX = [-82.0, 86.0, 0.0];
  const offsetsY = [26.0, 22.0, -84.0];
  const baseAngles = [2.52, 0.62, -1.58];

  for (let i: u32 = 0; i < COMMAND_COUNT; i += 1) {
    const offset = outputPtr + <usize>(i * SPAWN_IMAGE_COMMAND_BYTES);
    const angle = <f32>baseAngles[i];
    const speed = <f32>148.0 + <f32>rangedFromSeed(seed, 2 + i, 0, 40);
    writeSpawnImage(
      offset,
      x + <f32>offsetsX[i],
      y + <f32>offsetsY[i],
      <f32>Math.cos(angle) * speed,
      <f32>Math.sin(angle) * speed - 22.0,
      0.0,
      132.0 + <f32>i * 18.0,
      0.94 + (<f32>rangedFromSeed(seed, 11 + i, 0, 28) / 100.0),
      <f32>signedFromSeed(seed, 16 + i, 20) * 0.01,
      0.92,
      colorFromSeed(seed ^ (0xC2B2AE35 * (i + 1))),
      0,
      720 + i * 70,
      i,
    );
  }

  return OUTPUT_BYTES;
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  return 0;
}
