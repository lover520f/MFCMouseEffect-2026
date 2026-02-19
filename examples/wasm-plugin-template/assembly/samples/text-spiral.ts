import {
  API_VERSION,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleClick,
  readClickTickMs,
  readClickX,
  readClickY,
  writeSpawnText,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

const COMMAND_COUNT: u32 = 3;
const OUTPUT_BYTES: u32 = SPAWN_TEXT_COMMAND_BYTES * COMMAND_COUNT;

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
  const baseVy = <f32>-72.0 - <f32>rangedFromSeed(seed, 2, 0, 26);

  for (let i: u32 = 0; i < COMMAND_COUNT; i += 1) {
    const offset = outputPtr + <usize>(i * SPAWN_TEXT_COMMAND_BYTES);
    const side: f32 = i == 0 ? <f32>-1.0 : i == 1 ? <f32>1.0 : <f32>0.0;
    const vx: f32 = side * (<f32>22.0 + <f32>rangedFromSeed(seed, 6 + i, 0, 18)) + <f32>signedFromSeed(seed, 9 + i, 4);
    const rot: f32 = side * <f32>0.18;
    writeSpawnText(
      offset,
      x,
      y,
      vx,
      baseVy - <f32>(i * 8),
      0.0,
      168.0,
      0.95 + <f32>i * 0.08,
      rot,
      1.0,
      colorFromSeed(seed ^ (0x9E3779B9 * (i + 1))),
      i * 20,
      640 + i * 70,
      (seed + i * 5) % 10,
    );
  }
  return OUTPUT_BYTES;
}
