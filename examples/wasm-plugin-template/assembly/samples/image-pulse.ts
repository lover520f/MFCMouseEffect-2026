import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  SPAWN_IMAGE_COMMAND_BYTES,
  canHandleClick,
  readClickButton,
  readClickTickMs,
  readClickX,
  readClickY,
  writeSpawnImage,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs } from "../common/random";

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
  if (!canHandleClick(inputLen, outputCap, SPAWN_IMAGE_COMMAND_BYTES)) {
    return 0;
  }

  const x = <f32>readClickX(inputPtr);
  const y = <f32>readClickY(inputPtr);
  const button = readClickButton(inputPtr);
  const seed = seedFromTickMs(readClickTickMs(inputPtr));

  let imageId: u32 = 0;
  if (button == BUTTON_RIGHT) {
    imageId = 1;
  } else if (button == BUTTON_MIDDLE) {
    imageId = 2;
  } else if (button != BUTTON_LEFT) {
    imageId = seed % 4;
  }

  writeSpawnImage(
    outputPtr,
    x,
    y,
    0.0,
    -20.0 - <f32>rangedFromSeed(seed, 3, 0, 20),
    0.0,
    70.0,
    1.0 + (<f32>rangedFromSeed(seed, 8, 0, 35) / 100.0),
    0.0,
    1.0,
    colorFromSeed(seed ^ 0x55CCAA11),
    0,
    540,
    imageId,
  );

  return SPAWN_IMAGE_COMMAND_BYTES;
}
