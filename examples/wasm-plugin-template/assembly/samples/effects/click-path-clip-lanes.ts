import {
  API_VERSION,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  EVENT_KIND_CLICK,
  PATH_STROKE_LINE_CAP_ROUND,
  PATH_STROKE_LINE_JOIN_ROUND,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  spawnPathStrokeCommandBytesWithSemanticsAndClip,
  spawnRibbonStripCommandBytesWithSemanticsAndClip,
  writePathStrokeNodeClose,
  writePathStrokeNodeCubicTo,
  writePathStrokeNodeMoveTo,
  writePathStrokeNodeQuadTo,
  writeSpawnPathStrokeHeaderWithSemanticsAndClip,
  writeSpawnRibbonStripHeaderWithSemanticsAndClip,
  writeSpawnRibbonStripPoint,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const PATH_NODE_COUNT: u32 = 4;
const RIBBON_POINT_COUNT: u32 = 6;
const PATH_COMMAND_BYTES: u32 = spawnPathStrokeCommandBytesWithSemanticsAndClip(PATH_NODE_COUNT);
const RIBBON_COMMAND_BYTES: u32 = spawnRibbonStripCommandBytesWithSemanticsAndClip(RIBBON_POINT_COUNT);
const TOTAL_COMMAND_BYTES: u32 = PATH_COMMAND_BYTES + RIBBON_COMMAND_BYTES;

function strokeColor(button: u8): u32 {
  if (button == BUTTON_MIDDLE) return 0xFF95F6FF;
  if (button == BUTTON_LEFT) return 0xFFFF8EDC;
  return 0xFFFFC87A;
}

function ribbonColor(button: u8): u32 {
  if (button == BUTTON_MIDDLE) return 0xFF728CFF;
  if (button == BUTTON_LEFT) return 0xFF55E5FF;
  return 0xFFFF8F68;
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
  if (!canHandleEvent(inputLen, outputCap, TOTAL_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const button = readEventButton(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));

  const pathRadius = <f32>(40.0 + <f32>rangedFromSeed(seed, 3, 0, 10));
  const pathClipWidth = <f32>(46.0 + <f32>rangedFromSeed(seed, 7, 0, 12));
  const pathClipHeight = <f32>(84.0 + <f32>rangedFromSeed(seed, 11, 0, 10));
  const pathClipLeft = <f32>(x - pathRadius - 18.0 + <f32>signedFromSeed(seed, 13, 5));
  const pathClipTop = <f32>(y - pathClipHeight * 0.5 + <f32>signedFromSeed(seed, 17, 5));

  writeSpawnPathStrokeHeaderWithSemanticsAndClip(
    outputPtr,
    PATH_NODE_COUNT,
    button == BUTTON_MIDDLE ? 8.0 : 10.0,
    0.92,
    strokeColor(button),
    strokeColor(button) & 0x00FFFFFF | 0x66000000,
    0,
    360,
    PATH_STROKE_LINE_JOIN_ROUND,
    PATH_STROKE_LINE_CAP_ROUND,
    BLEND_MODE_SCREEN,
    32,
    1801,
    pathClipLeft,
    pathClipTop,
    pathClipWidth,
    pathClipHeight,
  );
  writePathStrokeNodeMoveTo(outputPtr, 0, <f32>(x - pathRadius), <f32>(y + 14.0));
  writePathStrokeNodeQuadTo(
    outputPtr,
    1,
    <f32>(x - pathRadius * 0.38),
    <f32>(y - pathRadius),
    <f32>(x + pathRadius * 0.22),
    <f32>(y - 10.0),
  );
  writePathStrokeNodeCubicTo(
    outputPtr,
    2,
    <f32>(x + pathRadius * 0.58),
    <f32>(y + 10.0),
    <f32>(x + pathRadius * 0.9),
    <f32>(y + pathRadius * 0.44),
    <f32>(x + pathRadius * 1.08),
    <f32>(y - 4.0),
  );
  writePathStrokeNodeClose(outputPtr, 3);

  const ribbonOffset = outputPtr + <usize>PATH_COMMAND_BYTES;
  const ribbonClipWidth = <f32>(108.0 + <f32>rangedFromSeed(seed, 19, 0, 20));
  const ribbonClipHeight = <f32>(54.0 + <f32>rangedFromSeed(seed, 23, 0, 14));
  const ribbonClipLeft = <f32>(x - ribbonClipWidth * 0.14 + <f32>signedFromSeed(seed, 29, 6));
  const ribbonClipTop = <f32>(y - ribbonClipHeight * 0.34 + <f32>signedFromSeed(seed, 31, 5));

  writeSpawnRibbonStripHeaderWithSemanticsAndClip(
    ribbonOffset,
    RIBBON_POINT_COUNT,
    0.84,
    7.0,
    ribbonColor(button),
    ribbonColor(button) & 0x00FFFFFF | 0x55000000,
    26,
    320,
    0,
    BLEND_MODE_SCREEN,
    18,
    1802,
    ribbonClipLeft,
    ribbonClipTop,
    ribbonClipWidth,
    ribbonClipHeight,
  );

  for (let index: u32 = 0; index < RIBBON_POINT_COUNT; index += 1) {
    const t = <f32>index / <f32>(RIBBON_POINT_COUNT - 1);
    const px = <f32>(x + 18.0 + t * 126.0 + <f32>signedFromSeed(seed, index * 37 + 5, 6));
    const py = <f32>(
      y + <f32>Math.sin(t * <f32>4.7) * <f32>18.0 - t * <f32>32.0
      + <f32>signedFromSeed(seed, index * 43 + 9, 5)
    );
    const widthPx = <f32>(24.0 - t * 13.0 + <f32>rangedFromSeed(seed, index * 41 + 15, 0, 4));
    writeSpawnRibbonStripPoint(ribbonOffset, index, px, py, widthPx);
  }

  return TOTAL_COMMAND_BYTES;
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
