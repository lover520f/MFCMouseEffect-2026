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

const COMMAND_COUNT: u32 = 4;
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
  const baseVx = <f32>signedFromSeed(seed, 3, 12);
  const baseVy = <f32>-70.0 - <f32>rangedFromSeed(seed, 5, 0, 24);

  for (let i: u32 = 0; i < COMMAND_COUNT; i += 1) {
    const offset = outputPtr + <usize>(i * SPAWN_TEXT_COMMAND_BYTES);
    const phase: f32 = (i % 2 == 0) ? <f32>-1.0 : <f32>1.0;
    writeSpawnText(
      offset,
      x,
      y,
      baseVx + phase * (<f32>18.0 + <f32>i * <f32>4.0),
      baseVy - <f32>i * 7.0,
      0.0,
      150.0 + <f32>i * 8.0,
      0.9 + <f32>i * 0.04,
      phase * <f32>0.08,
      1.0,
      colorFromSeed(seed ^ (0x45D9F3B * (i + 7))),
      i * 28,
      560 + i * 60,
      (seed >> (i + 1)) % 12,
    );
  }
  return OUTPUT_BYTES;
}
