import {
  API_VERSION,
  BLEND_MODE_ADD,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnPathFillCommandBytesWithSemantics,
} from "../../common/abi";
import {
  PATH_BADGE_NODE_COUNT,
  PATH_BADGE_SPARK_NODE_COUNT,
  emitPathBadge,
  emitPathBadgeSpark,
} from "../../common/path-badge";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const BADGE_COMMAND_BYTES: u32 = spawnPathFillCommandBytesWithSemantics(PATH_BADGE_NODE_COUNT);
const SPARK_COMMAND_BYTES: u32 = spawnPathFillCommandBytesWithSemantics(PATH_BADGE_SPARK_NODE_COUNT);

function badgeFill(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFB7E58;
  if (button == BUTTON_MIDDLE) return 0xFFF4F0FF;
  return 0xFF3DE7D0;
}

function badgeGlow(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0x78FF7E58;
  if (button == BUTTON_MIDDLE) return 0x889A82FF;
  return 0x7834D6FF;
}

function accentFill(button: u8): u32 {
  if (button == BUTTON_MIDDLE) return 0xFFEFD8FF;
  if (button == BUTTON_RIGHT) return 0xFFFFE3D9;
  return 0xFFDDFBF4;
}

function blendMode(button: u8): u32 {
  return button == BUTTON_MIDDLE ? BLEND_MODE_ADD : BLEND_MODE_SCREEN;
}

function emitBadge(outputPtr: usize, x: f32, y: f32, seed: u32, button: u8): void {
  const radius: f32 = 24.0 + <f32>rangedFromSeed(seed, 5, 0, 10);
  const inner: f32 = radius * 0.44;
  const skew: f32 = <f32>signedFromSeed(seed, 11, 4);
  emitPathBadge(
    outputPtr,
    x,
    y,
    radius,
    inner,
    skew,
    0.94,
    button == BUTTON_MIDDLE ? 10.0 : 12.0,
    badgeFill(button),
    badgeGlow(button),
    0,
    340,
    blendMode(button),
    24,
    2,
  );
}

function emitSpark(outputPtr: usize, x: f32, y: f32, seed: u32, button: u8): void {
  const spread: f32 = 18.0 + <f32>rangedFromSeed(seed, 17, 0, 12);
  const lift: f32 = 22.0 + <f32>rangedFromSeed(seed, 23, 0, 10);
  const tilt: f32 = <f32>signedFromSeed(seed, 29, 8);
  emitPathBadgeSpark(
    outputPtr,
    x,
    y,
    spread,
    lift,
    tilt,
    0.70,
    8.0,
    accentFill(button),
    badgeGlow(button),
    34,
    220,
    blendMode(button),
    18,
    2,
  );
}

export function mfx_plugin_get_api_version(): u32 {
  return API_VERSION;
}

export function mfx_plugin_reset(): void {}

export function mfx_plugin_on_input(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  if (!canHandleEvent(inputLen, outputCap, BADGE_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const button = readEventButton(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));

  let written: u32 = 0;
  emitBadge(outputPtr, x, y, seed ^ 0x5163A17D, button);
  written += BADGE_COMMAND_BYTES;

  if (outputCap < written + SPARK_COMMAND_BYTES) {
    return written;
  }

  emitSpark(
    outputPtr + <usize>written,
    x + <f32>signedFromSeed(seed, 37, 10),
    y - 8.0,
    seed ^ 0x9E3779B9,
    button,
  );
  written += SPARK_COMMAND_BYTES;
  return written;
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  inputPtr;
  inputLen;
  outputPtr;
  outputCap;
  return 0;
}
