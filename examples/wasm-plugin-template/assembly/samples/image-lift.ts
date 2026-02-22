import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  SPAWN_IMAGE_COMMAND_BYTES,
  canHandleClickEvent,
  readEventButton,
  readEventTickMs,
  readEventX,
  readEventY,
  writeSpawnImage,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs } from "../common/random";

const COMMAND_COUNT: u32 = 2;
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
  const button = readEventButton(inputPtr);

  let baseId: u32 = 0;
  if (button == BUTTON_RIGHT) {
    baseId = 1;
  } else if (button == BUTTON_MIDDLE) {
    baseId = 2;
  } else if (button != BUTTON_LEFT) {
    baseId = 3;
  }

  writeSpawnImage(
    outputPtr,
    x,
    y,
    -72.0,
    -165.0 - <f32>rangedFromSeed(seed, 6, 0, 46),
    0.0,
    110.0,
    1.0,
    -0.08,
    1.0,
    colorFromSeed(seed ^ 0x9E21B43),
    0,
    650,
    baseId,
  );

  writeSpawnImage(
    outputPtr + <usize>SPAWN_IMAGE_COMMAND_BYTES,
    x,
    y,
    72.0,
    -172.0 - <f32>rangedFromSeed(seed, 9, 0, 42),
    0.0,
    120.0,
    1.1,
    0.08,
    0.95,
    colorFromSeed(seed ^ 0x7F4A7C15),
    35,
    700,
    baseId + 1,
  );

  return OUTPUT_BYTES;
}
