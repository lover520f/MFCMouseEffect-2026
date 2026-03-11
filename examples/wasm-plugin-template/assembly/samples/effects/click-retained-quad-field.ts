import {
  API_VERSION,
  BLEND_MODE_ADD,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  REMOVE_QUAD_FIELD_COMMAND_BYTES,
  UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  upsertQuadFieldCommandBytesWithSemanticsAndClip,
  writeRemoveQuadField,
  writeUpsertQuadFieldHeaderWithSemanticsAndClip,
  writeUpsertQuadFieldItem,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const ITEM_COUNT: u32 = 4;
const FIELD_ID: u32 = 5107;
const FIELD_GROUP_ID: u32 = 5108;
const UPSERT_COMMAND_BYTES: u32 = upsertQuadFieldCommandBytesWithSemanticsAndClip(ITEM_COUNT);
const FIELD_TTL_MS: u32 = 1200;
const TAU: f32 = <f32>6.283185307179586;

function fieldTint(button: u8, index: u32): u32 {
  if (button == BUTTON_MIDDLE) {
    return (index & 1) == 0 ? 0xFF7AF5FF : 0xFFFFC86B;
  }
  return (index & 1) == 0 ? 0xFF5BF2FF : 0xFFFF72D4;
}

function emitField(outputPtr: usize, x: f32, y: f32, seed: u32, button: u8): void {
  const clipWidthPx: f32 = button == BUTTON_MIDDLE ? 84.0 : 72.0;
  const clipHeightPx: f32 = button == BUTTON_MIDDLE ? 124.0 : 110.0;
  writeUpsertQuadFieldHeaderWithSemanticsAndClip(
    outputPtr,
    ITEM_COUNT,
    FIELD_TTL_MS,
    FIELD_ID,
    UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    24,
    FIELD_GROUP_ID,
    x - clipWidthPx * 0.5,
    y - clipHeightPx * 0.5,
    clipWidthPx,
    clipHeightPx,
  );

  for (let index: u32 = 0; index < ITEM_COUNT; index += 1) {
    const angle = (<f32>index / <f32>ITEM_COUNT) * TAU;
    const radius = <f32>42.0 + <f32>rangedFromSeed(seed, index * 13 + 7, 0, 24);
    const widthPx = <f32>34.0 + <f32>rangedFromSeed(seed, index * 19 + 9, 0, 24);
    const heightPx = <f32>26.0 + <f32>rangedFromSeed(seed, index * 23 + 11, 0, 20);
    const localX = x + <f32>Math.cos(angle) * radius + <f32>signedFromSeed(seed, index * 29 + 13, 10);
    const localY = y + <f32>Math.sin(angle) * radius + <f32>signedFromSeed(seed, index * 31 + 17, 10);
    const rotation = angle * <f32>0.65 + <f32>signedFromSeed(seed, index * 37 + 19, 35) * <f32>0.017453292;
    const vx = <f32>Math.cos(angle) * (<f32>18.0 + <f32>rangedFromSeed(seed, index * 41 + 23, 0, 26));
    const vy = <f32>Math.sin(angle) * (<f32>18.0 + <f32>rangedFromSeed(seed, index * 43 + 29, 0, 26)) - <f32>16.0;
    const ax = <f32>signedFromSeed(seed, index * 47 + 31, 10);
    const ay = button == BUTTON_MIDDLE ? <f32>18.0 : <f32>30.0;
    const tile = index & 1;
    writeUpsertQuadFieldItem(
      outputPtr,
      index,
      localX,
      localY,
      widthPx,
      heightPx,
      <f32>0.86,
      rotation,
      fieldTint(button, index),
      index % 3,
      <f32>tile * <f32>0.5,
      index < 2 ? <f32>0.0 : <f32>0.5,
      (<f32>tile + <f32>1.0) * <f32>0.5,
      index < 2 ? <f32>0.5 : <f32>1.0,
      vx,
      vy,
      ax,
      ay,
    );
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
  if (!canHandleEvent(inputLen, outputCap, REMOVE_QUAD_FIELD_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const button = readEventButton(inputPtr);
  if (button == BUTTON_RIGHT) {
    if (outputCap < REMOVE_QUAD_FIELD_COMMAND_BYTES) {
      return 0;
    }
    writeRemoveQuadField(outputPtr, FIELD_ID);
    return REMOVE_QUAD_FIELD_COMMAND_BYTES;
  }

  if ((button != BUTTON_LEFT && button != BUTTON_MIDDLE) || outputCap < UPSERT_COMMAND_BYTES) {
    return 0;
  }

  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  emitField(outputPtr, <f32>readEventX(inputPtr), <f32>readEventY(inputPtr), seed, button);
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
