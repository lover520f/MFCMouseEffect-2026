const API_VERSION: u32 = 1;
const CLICK_INPUT_BYTES: u32 = 20;
const COMMAND_KIND_SPAWN_TEXT: u16 = 1;
const SPAWN_TEXT_COMMAND_BYTES: u32 = 56;

const BUTTON_LEFT: u8 = 1;
const BUTTON_RIGHT: u8 = 2;

function MakeColor(seed: u32): u32 {
  const r: u32 = (seed >> 0) & 0xff;
  const g: u32 = (seed >> 8) & 0xff;
  const b: u32 = (seed >> 16) & 0xff;
  return 0xff000000 | (r << 16) | (g << 8) | b;
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {}

export function mfx_plugin_on_click(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32
): u32 {
  if (inputLen < CLICK_INPUT_BYTES || outputCap < SPAWN_TEXT_COMMAND_BYTES) {
    return 0;
  }

  const x: i32 = load<i32>(inputPtr + 0);
  const y: i32 = load<i32>(inputPtr + 4);
  const button: u8 = load<u8>(inputPtr + 8);
  const tickMs: u64 = load<u64>(inputPtr + 12);
  const seed: u32 = <u32>(tickMs & 0xffffffff);

  let vx: f32 = 24.0 + <f32>(seed % 22);
  let vy: f32 = -120.0 - <f32>((seed >> 5) % 36);
  if (button == BUTTON_RIGHT) {
    vx = -vx;
    vy = -96.0 - <f32>((seed >> 9) % 28);
  } else if (button != BUTTON_LEFT) {
    vx = <f32>((seed % 31) - 15);
    vy = -84.0 - <f32>((seed >> 2) % 18);
  }

  const color: u32 = MakeColor(seed);

  store<u16>(outputPtr + 0, COMMAND_KIND_SPAWN_TEXT);
  store<u16>(outputPtr + 2, <u16>SPAWN_TEXT_COMMAND_BYTES);
  store<f32>(outputPtr + 4, <f32>x);
  store<f32>(outputPtr + 8, <f32>y);
  store<f32>(outputPtr + 12, vx);
  store<f32>(outputPtr + 16, vy);
  store<f32>(outputPtr + 20, 0.0);
  store<f32>(outputPtr + 24, 240.0);
  store<f32>(outputPtr + 28, 1.0);
  store<f32>(outputPtr + 32, 0.0);
  store<f32>(outputPtr + 36, 1.0);
  store<u32>(outputPtr + 40, color);
  store<u32>(outputPtr + 44, 0);
  store<u32>(outputPtr + 48, 650);
  store<u32>(outputPtr + 52, 0);
  return SPAWN_TEXT_COMMAND_BYTES;
}
