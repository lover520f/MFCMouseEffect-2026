import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_RIGHT,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleClick,
  readClickButton,
  readClickTickMs,
  readClickX,
  readClickY,
  writeSpawnText,
} from "../common/abi";
import { colorFromSeed, rangedFromSeed, seedFromTickMs, signedFromSeed } from "../common/random";

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
  if (!canHandleClick(inputLen, outputCap, SPAWN_TEXT_COMMAND_BYTES)) {
    return 0;
  }

  const x = <f32>readClickX(inputPtr);
  const y = <f32>readClickY(inputPtr);
  const button = readClickButton(inputPtr);
  const seed = seedFromTickMs(readClickTickMs(inputPtr));

  let vx: f32 = 20.0 + <f32>rangedFromSeed(seed, 2, 0, 30);
  let vy: f32 = -90.0 - <f32>rangedFromSeed(seed, 7, 0, 48);
  if (button == BUTTON_RIGHT) {
    vx = -vx;
  } else if (button != BUTTON_LEFT) {
    vx = <f32>signedFromSeed(seed, 4, 14);
    vy = -70.0 - <f32>rangedFromSeed(seed, 10, 0, 18);
  }

  writeSpawnText(
    outputPtr,
    x,
    y,
    vx,
    vy,
    0.0,
    220.0,
    1.0,
    0.0,
    1.0,
    colorFromSeed(seed),
    0,
    680,
    seed % 8,
  );
  return SPAWN_TEXT_COMMAND_BYTES;
}
