import {
  API_VERSION,
  BLEND_MODE_ADD,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  GROUP_CLIP_MASK_SHAPE_ELLIPSE,
  GROUP_CLIP_MASK_SHAPE_ROUND_RECT,
  REMOVE_GROUP_COMMAND_BYTES,
  UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS,
  UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
  UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
  UPSERT_RIBBON_TRAIL_FLAG_CLOSED,
  UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES_WITH_MASK_TAIL,
  canHandleEvent,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  upsertQuadFieldCommandBytesWithSemantics,
  upsertRibbonTrailCommandBytesWithSemantics,
  writeRemoveGroup,
  writeUpsertGlowEmitterWithSemantics,
  writeUpsertGroupClipRectWithMaskTail,
  writeUpsertQuadFieldHeaderWithSemantics,
  writeUpsertQuadFieldItem,
  writeUpsertRibbonTrailHeaderWithSemantics,
  writeUpsertRibbonTrailPoint,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const GROUP_ID: u32 = 7710;
const GLOW_ID: u32 = 7711;
const TRAIL_ID: u32 = 7712;
const FIELD_ID: u32 = 7713;
const TRAIL_POINT_COUNT: u32 = 5;
const FIELD_ITEM_COUNT: u32 = 5;

const CLIP_BYTES: u32 = UPSERT_GROUP_CLIP_RECT_COMMAND_BYTES_WITH_MASK_TAIL;
const GLOW_BYTES: u32 = UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS;
const TRAIL_BYTES: u32 = upsertRibbonTrailCommandBytesWithSemantics(TRAIL_POINT_COUNT);
const FIELD_BYTES: u32 = upsertQuadFieldCommandBytesWithSemantics(FIELD_ITEM_COUNT);
const LEFT_TOTAL_BYTES: u32 = CLIP_BYTES + GLOW_BYTES + TRAIL_BYTES + FIELD_BYTES;

function emitGroupMask(
  outputPtr: usize,
  x: f32,
  y: f32,
  widthPx: f32,
  heightPx: f32,
  shapeKind: u32,
  cornerRadiusPx: f32,
): void {
  writeUpsertGroupClipRectWithMaskTail(
    outputPtr,
    x - widthPx * <f32>0.5,
    y - heightPx * <f32>0.5,
    widthPx,
    heightPx,
    GROUP_ID,
    shapeKind,
    cornerRadiusPx,
    true,
  );
}

function emitGlow(outputPtr: usize, x: f32, y: f32): void {
  writeUpsertGlowEmitterWithSemantics(
    outputPtr,
    x,
    y,
    <f32>116.0,
    <f32>-1.5707963,
    <f32>0.94,
    <f32>92.0,
    <f32>182.0,
    <f32>3.0,
    <f32>8.0,
    <f32>0.18,
    <f32>0.84,
    0xFF68F5FF,
    <f32>0.0,
    <f32>164.0,
    GLOW_ID,
    1400,
    920,
    104,
    UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    18,
    GROUP_ID,
  );
}

function emitRibbon(outputPtr: usize, x: f32, y: f32): void {
  writeUpsertRibbonTrailHeaderWithSemantics(
    outputPtr,
    TRAIL_POINT_COUNT,
    <f32>0.92,
    <f32>10.0,
    0xFF5AF2FF,
    0x665AF2FF,
    1080,
    TRAIL_ID,
    UPSERT_RIBBON_TRAIL_FLAG_CLOSED,
    BLEND_MODE_SCREEN,
    22,
    GROUP_ID,
  );
  for (let index: u32 = 0; index < TRAIL_POINT_COUNT; index += 1) {
    const progress = <f32>index / <f32>(TRAIL_POINT_COUNT - 1);
    const px = x + (progress - <f32>0.5) * <f32>108.0;
    const py = y - <f32>20.0 + <f32>Math.sin(progress * <f32>6.2831853) * <f32>24.0;
    const widthPx = <f32>14.0 + (<f32>1.0 - <f32>Math.abs(progress - <f32>0.5) * <f32>2.0) * <f32>20.0;
    writeUpsertRibbonTrailPoint(outputPtr, index, px, py, widthPx);
  }
}

function emitQuadField(outputPtr: usize, x: f32, y: f32, seed: u32): void {
  writeUpsertQuadFieldHeaderWithSemantics(
    outputPtr,
    FIELD_ITEM_COUNT,
    1240,
    FIELD_ID,
    UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    24,
    GROUP_ID,
  );

  for (let index: u32 = 0; index < FIELD_ITEM_COUNT; index += 1) {
    const angle = (<f32>index / <f32>FIELD_ITEM_COUNT) * <f32>6.2831853;
    const radius = <f32>30.0 + <f32>rangedFromSeed(seed, index * 7 + 3, 0, 28);
    const localX = x + <f32>Math.cos(angle) * radius;
    const localY = y + <f32>Math.sin(angle) * radius;
    const widthPx = <f32>26.0 + <f32>rangedFromSeed(seed, index * 13 + 5, 0, 28);
    const heightPx = <f32>22.0 + <f32>rangedFromSeed(seed, index * 17 + 9, 0, 18);
    const tile = index % 3;
    writeUpsertQuadFieldItem(
      outputPtr,
      index,
      localX,
      localY,
      widthPx,
      heightPx,
      <f32>0.84,
      angle * <f32>0.55,
      0xFF6FEFFF,
      tile,
      tile == 0 ? <f32>0.0 : tile == 1 ? <f32>0.5 : <f32>0.0,
      tile == 2 ? <f32>0.5 : <f32>0.0,
      tile == 0 ? <f32>1.0 : tile == 1 ? <f32>1.0 : <f32>0.5,
      tile == 2 ? <f32>1.0 : <f32>0.5,
      <f32>signedFromSeed(seed, index * 19 + 11, 18),
      <f32>signedFromSeed(seed, index * 23 + 13, 18) - <f32>18.0,
      <f32>signedFromSeed(seed, index * 29 + 17, 5),
      <f32>18.0,
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
  if (!canHandleEvent(inputLen, outputCap, REMOVE_GROUP_COMMAND_BYTES)) {
    return 0;
  }
  if (readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const button = readEventButton(inputPtr);
  if (button == BUTTON_RIGHT) {
    if (outputCap < REMOVE_GROUP_COMMAND_BYTES) {
      return 0;
    }
    writeRemoveGroup(outputPtr, GROUP_ID);
    return REMOVE_GROUP_COMMAND_BYTES;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);

  if (button == BUTTON_MIDDLE) {
    if (outputCap < CLIP_BYTES) {
      return 0;
    }
    emitGroupMask(
      outputPtr,
      x + <f32>44.0,
      y,
      <f32>112.0,
      <f32>92.0,
      GROUP_CLIP_MASK_SHAPE_ELLIPSE,
      <f32>0.0,
    );
    return CLIP_BYTES;
  }

  if (button != BUTTON_LEFT || outputCap < LEFT_TOTAL_BYTES) {
    return 0;
  }

  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  emitGroupMask(
    outputPtr,
    x,
    y,
    <f32>180.0,
    <f32>132.0,
    GROUP_CLIP_MASK_SHAPE_ROUND_RECT,
    <f32>26.0,
  );
  emitGlow(outputPtr + <usize>CLIP_BYTES, x, y);
  emitRibbon(outputPtr + <usize>(CLIP_BYTES + GLOW_BYTES), x, y);
  emitQuadField(outputPtr + <usize>(CLIP_BYTES + GLOW_BYTES + TRAIL_BYTES), x, y, seed);
  return LEFT_TOTAL_BYTES;
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
