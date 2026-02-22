import {
  API_VERSION,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleClickEvent,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnText,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

const BURST_COMMAND_COUNT: u32 = 2;
const BURST_OUTPUT_BYTES: u32 = SPAWN_TEXT_COMMAND_BYTES * BURST_COMMAND_COUNT;

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
  if (!canHandleClickEvent(inputPtr, inputLen, outputCap, BURST_OUTPUT_BYTES)) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const baseVy: f32 = <f32>-76.0 - <f32>rangedFromSeed(seed, 5, 0, 35);
  const drift: f32 = <f32>signedFromSeed(seed, 9, 18);

  writeSpawnText(
    outputPtr,
    x,
    y,
    <f32>-34.0 + drift,
    baseVy,
    <f32>0.0,
    <f32>180.0,
    <f32>1.0,
    <f32>-0.16,
    <f32>1.0,
    colorFromSeed(seed ^ 0x2D7EA911),
    0,
    620,
    seed % 8,
  );

  writeSpawnText(
    outputPtr + <usize>SPAWN_TEXT_COMMAND_BYTES,
    x,
    y,
    <f32>34.0 - drift,
    baseVy - <f32>14.0,
    <f32>0.0,
    <f32>180.0,
    <f32>1.0,
    <f32>0.16,
    <f32>1.0,
    colorFromSeed(seed ^ 0xA17D2C59),
    30,
    700,
    (seed >> 4) % 8,
  );
  return BURST_OUTPUT_BYTES;
}
