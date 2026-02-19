import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  SPAWN_IMAGE_COMMAND_BYTES,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleClick,
  readClickButton,
  readClickTickMs,
  readClickX,
  readClickY,
  writeSpawnImage,
  writeSpawnText,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

const OUTPUT_BYTES: u32 = SPAWN_TEXT_COMMAND_BYTES + SPAWN_IMAGE_COMMAND_BYTES;

function textIdByButton(button: u8, seed: u32): u32 {
  if (button == BUTTON_LEFT) {
    return 0;
  }
  if (button == BUTTON_RIGHT) {
    return 1;
  }
  if (button == BUTTON_MIDDLE) {
    return 2;
  }
  return 3 + (seed % 4);
}

function imageIdByButton(button: u8): u32 {
  if (button == BUTTON_LEFT) {
    return 0;
  }
  if (button == BUTTON_RIGHT) {
    return 1;
  }
  if (button == BUTTON_MIDDLE) {
    return 2;
  }
  return 3;
}

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
  const button = readClickButton(inputPtr);
  const seed = seedFromTickMs(readClickTickMs(inputPtr));

  writeSpawnText(
    outputPtr,
    x,
    y,
    <f32>signedFromSeed(seed, 3, 14),
    -84.0 - <f32>rangedFromSeed(seed, 8, 0, 22),
    0.0,
    170.0,
    1.0,
    0.0,
    1.0,
    colorFromSeed(seed ^ 0xC15A31EF),
    0,
    650,
    textIdByButton(button, seed),
  );

  writeSpawnImage(
    outputPtr + <usize>SPAWN_TEXT_COMMAND_BYTES,
    x,
    y,
    <f32>signedFromSeed(seed, 10, 84),
    -170.0 - <f32>rangedFromSeed(seed, 14, 0, 46),
    0.0,
    120.0,
    0.95 + (<f32>rangedFromSeed(seed, 18, 0, 25) / 100.0),
    <f32>signedFromSeed(seed, 23, 10) * 0.01,
    0.95,
    colorFromSeed(seed ^ 0x92BF6513),
    20,
    600,
    imageIdByButton(button),
  );

  return OUTPUT_BYTES;
}
