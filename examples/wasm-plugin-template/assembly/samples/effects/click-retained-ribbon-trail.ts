import {
  API_VERSION,
  BLEND_MODE_ADD,
  BUTTON_LEFT,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  REMOVE_RIBBON_TRAIL_COMMAND_BYTES,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  upsertRibbonTrailCommandBytesWithSemanticsAndClip,
  writeRemoveRibbonTrail,
  writeUpsertRibbonTrailHeaderWithSemanticsAndClip,
  writeUpsertRibbonTrailPoint,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const POINT_COUNT: u32 = 8;
const TRAIL_ID: u32 = 4107;
const TRAIL_GROUP_ID: u32 = 4108;
const UPSERT_COMMAND_BYTES: u32 = upsertRibbonTrailCommandBytesWithSemanticsAndClip(POINT_COUNT);
const TRAIL_TTL_MS: u32 = 1200;

function colorFor(button: u8, layer: u32): u32 {
  if (button == BUTTON_RIGHT) {
    return layer == 0 ? 0xFFFFB35C : 0xFFFF6A4A;
  }
  return layer == 0 ? 0xFF6DF0FF : 0xFF7F86FF;
}

function emitTrail(outputPtr: usize, x: f32, y: f32, seed: u32, button: u8): void {
  writeUpsertRibbonTrailHeaderWithSemanticsAndClip(
    outputPtr,
    POINT_COUNT,
    <f32>0.90,
    <f32>10.0,
    colorFor(button, 0),
    colorFor(button, 1) & 0x00FFFFFF | 0x55000000,
    TRAIL_TTL_MS,
    TRAIL_ID,
    0,
    BLEND_MODE_ADD,
    28,
    TRAIL_GROUP_ID,
    x - 76.0,
    y - 42.0,
    152.0,
    96.0,
  );

  for (let index: u32 = 0; index < POINT_COUNT; index += 1) {
    const t = <f32>index / <f32>(POINT_COUNT - 1);
    const arc = <f32>Math.sin(t * <f32>3.1415926) * <f32>52.0;
    const driftX = t * <f32>164.0 - <f32>82.0 + <f32>signedFromSeed(seed, index * 17 + 5, 10);
    const driftY = -t * <f32>96.0 + arc + <f32>signedFromSeed(seed, index * 23 + 11, 8);
    const widthPx = (<f32>42.0 - t * <f32>24.0)
      + <f32>rangedFromSeed(seed, index * 31 + 19, 0, 6);
    writeUpsertRibbonTrailPoint(outputPtr, index, x + driftX, y + driftY, widthPx);
  }
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
  if (!canHandleEvent(inputLen, outputCap, REMOVE_RIBBON_TRAIL_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const button = readEventButton(inputPtr);
  if (button == BUTTON_RIGHT) {
    if (outputCap < REMOVE_RIBBON_TRAIL_COMMAND_BYTES) {
      return 0;
    }
    writeRemoveRibbonTrail(outputPtr, TRAIL_ID);
    return REMOVE_RIBBON_TRAIL_COMMAND_BYTES;
  }

  if (button != BUTTON_LEFT || outputCap < UPSERT_COMMAND_BYTES) {
    return 0;
  }

  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  emitTrail(outputPtr, <f32>readEventX(inputPtr), <f32>readEventY(inputPtr), seed, button);
  return UPSERT_COMMAND_BYTES;
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
