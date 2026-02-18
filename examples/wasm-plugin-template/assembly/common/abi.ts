export const API_VERSION: u32 = 1;

export const CLICK_INPUT_BYTES: u32 = 20;

export const COMMAND_KIND_SPAWN_TEXT: u16 = 1;
export const COMMAND_KIND_SPAWN_IMAGE: u16 = 2;

export const SPAWN_TEXT_COMMAND_BYTES: u32 = 56;
export const SPAWN_IMAGE_COMMAND_BYTES: u32 = 56;

export const BUTTON_LEFT: u8 = 1;
export const BUTTON_RIGHT: u8 = 2;
export const BUTTON_MIDDLE: u8 = 3;

export function canHandleClick(inputLen: u32, outputCap: u32, minOutputBytes: u32): bool {
  return inputLen >= CLICK_INPUT_BYTES && outputCap >= minOutputBytes;
}

export function readClickX(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 0);
}

export function readClickY(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 4);
}

export function readClickButton(inputPtr: usize): u8 {
  return load<u8>(inputPtr + 8);
}

export function readClickTickMs(inputPtr: usize): u64 {
  return load<u64>(inputPtr + 12);
}

export function writeSpawnText(
  outputPtr: usize,
  x: f32,
  y: f32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
  scale: f32,
  rotation: f32,
  alpha: f32,
  colorRgba: u32,
  delayMs: u32,
  lifeMs: u32,
  textId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_TEXT);
  store<u16>(outputPtr + 2, <u16>SPAWN_TEXT_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, vx);
  store<f32>(outputPtr + 16, vy);
  store<f32>(outputPtr + 20, ax);
  store<f32>(outputPtr + 24, ay);
  store<f32>(outputPtr + 28, scale);
  store<f32>(outputPtr + 32, rotation);
  store<f32>(outputPtr + 36, alpha);
  store<u32>(outputPtr + 40, colorRgba);
  store<u32>(outputPtr + 44, delayMs);
  store<u32>(outputPtr + 48, lifeMs);
  store<u32>(outputPtr + 52, textId);
}

export function writeSpawnImage(
  outputPtr: usize,
  x: f32,
  y: f32,
  vx: f32,
  vy: f32,
  ax: f32,
  ay: f32,
  scale: f32,
  rotation: f32,
  alpha: f32,
  tintRgba: u32,
  delayMs: u32,
  lifeMs: u32,
  imageId: u32,
): void {
  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_IMAGE);
  store<u16>(outputPtr + 2, <u16>SPAWN_IMAGE_COMMAND_BYTES);
  store<f32>(outputPtr + 4, x);
  store<f32>(outputPtr + 8, y);
  store<f32>(outputPtr + 12, vx);
  store<f32>(outputPtr + 16, vy);
  store<f32>(outputPtr + 20, ax);
  store<f32>(outputPtr + 24, ay);
  store<f32>(outputPtr + 28, scale);
  store<f32>(outputPtr + 32, rotation);
  store<f32>(outputPtr + 36, alpha);
  store<u32>(outputPtr + 40, tintRgba);
  store<u32>(outputPtr + 44, delayMs);
  store<u32>(outputPtr + 48, lifeMs);
  store<u32>(outputPtr + 52, imageId);
}
