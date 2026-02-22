export const API_VERSION: u32 = 1;

export const EVENT_INPUT_BYTES: u32 = 28;

export const COMMAND_KIND_SPAWN_TEXT: u16 = 1;
export const COMMAND_KIND_SPAWN_IMAGE: u16 = 2;

export const SPAWN_TEXT_COMMAND_BYTES: u32 = 56;
export const SPAWN_IMAGE_COMMAND_BYTES: u32 = 56;

export const BUTTON_LEFT: u8 = 1;
export const BUTTON_RIGHT: u8 = 2;
export const BUTTON_MIDDLE: u8 = 3;

export const EVENT_KIND_CLICK: u8 = 1;
export const EVENT_KIND_MOVE: u8 = 2;
export const EVENT_KIND_SCROLL: u8 = 3;
export const EVENT_KIND_HOLD_START: u8 = 4;
export const EVENT_KIND_HOLD_UPDATE: u8 = 5;
export const EVENT_KIND_HOLD_END: u8 = 6;
export const EVENT_KIND_HOVER_START: u8 = 7;
export const EVENT_KIND_HOVER_END: u8 = 8;

export const EVENT_FLAG_SCROLL_HORIZONTAL: u8 = 0x01;

export function canHandleEvent(inputLen: u32, outputCap: u32, minOutputBytes: u32): bool {
  return inputLen >= EVENT_INPUT_BYTES && outputCap >= minOutputBytes;
}

export function canHandleClickEvent(
  inputPtr: usize,
  inputLen: u32,
  outputCap: u32,
  minOutputBytes: u32,
): bool {
  return canHandleEvent(inputLen, outputCap, minOutputBytes)
    && readEventKind(inputPtr) == EVENT_KIND_CLICK;
}

export function readEventX(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 0);
}

export function readEventY(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 4);
}

export function readEventDelta(inputPtr: usize): i32 {
  return load<i32>(inputPtr + 8);
}

export function readEventHoldMs(inputPtr: usize): u32 {
  return load<u32>(inputPtr + 12);
}

export function readEventKind(inputPtr: usize): u8 {
  return load<u8>(inputPtr + 16);
}

export function readEventButton(inputPtr: usize): u8 {
  return load<u8>(inputPtr + 17);
}

export function readEventFlags(inputPtr: usize): u8 {
  return load<u8>(inputPtr + 18);
}

export function readEventTickMs(inputPtr: usize): u64 {
  return load<u64>(inputPtr + 20);
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
