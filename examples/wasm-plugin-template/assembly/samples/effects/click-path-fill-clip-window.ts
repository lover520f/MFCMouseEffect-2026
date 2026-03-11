import {
  API_VERSION,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  PATH_FILL_RULE_EVEN_ODD,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnPathFillCommandBytesWithSemanticsAndClip,
  writePathStrokeNodeClose,
  writePathStrokeNodeLineTo,
  writePathStrokeNodeMoveTo,
  writeSpawnPathFillHeaderWithSemanticsAndClip,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const NODE_COUNT: u32 = 10;
const COMMAND_BYTES: u32 = spawnPathFillCommandBytesWithSemanticsAndClip(NODE_COUNT);

function outerFill(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFFB07A;
  if (button == BUTTON_MIDDLE) return 0xFFF0E2FF;
  return 0xFF59F0D5;
}

function glowColor(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0x88FF9B74;
  if (button == BUTTON_MIDDLE) return 0x88977CFF;
  return 0x7837DFFF;
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
  if (!canHandleEvent(inputLen, outputCap, COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const button = readEventButton(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const radius: f32 = 44.0 + <f32>rangedFromSeed(seed, 5, 0, 10);
  const inner: f32 = radius * 0.42;
  const skew: f32 = <f32>signedFromSeed(seed, 11, 8);
  const clipWidthPx: f32 = 40.0 + <f32>rangedFromSeed(seed, 17, 0, 18);
  const clipHeightPx: f32 = radius * 2.0 + 18.0;
  const clipLeftPx: f32 = x - clipWidthPx * 0.5 + <f32>signedFromSeed(seed, 23, 8);
  const clipTopPx: f32 = y - clipHeightPx * 0.5 + <f32>signedFromSeed(seed, 29, 6);

  writeSpawnPathFillHeaderWithSemanticsAndClip(
    outputPtr,
    NODE_COUNT,
    0.92,
    button == BUTTON_MIDDLE ? 10.0 : 12.0,
    outerFill(button),
    glowColor(button),
    0,
    380,
    PATH_FILL_RULE_EVEN_ODD,
    BLEND_MODE_SCREEN,
    button == BUTTON_LEFT ? 22 : 30,
    1202,
    clipLeftPx,
    clipTopPx,
    clipWidthPx,
    clipHeightPx,
  );

  writePathStrokeNodeMoveTo(outputPtr, 0, x, y - radius);
  writePathStrokeNodeLineTo(outputPtr, 1, x + radius + skew, y);
  writePathStrokeNodeLineTo(outputPtr, 2, x, y + radius);
  writePathStrokeNodeLineTo(outputPtr, 3, x - radius + skew, y);
  writePathStrokeNodeClose(outputPtr, 4);

  writePathStrokeNodeMoveTo(outputPtr, 5, x, y - inner);
  writePathStrokeNodeLineTo(outputPtr, 6, x + inner, y);
  writePathStrokeNodeLineTo(outputPtr, 7, x, y + inner);
  writePathStrokeNodeLineTo(outputPtr, 8, x - inner, y);
  writePathStrokeNodeClose(outputPtr, 9);

  return COMMAND_BYTES;
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
