import {
  API_VERSION,
  BLEND_MODE_ADD,
  BLEND_MODE_NORMAL,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  REMOVE_GROUP_COMMAND_BYTES,
  UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS,
  UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
  UPSERT_GROUP_LAYER_COMMAND_BYTES,
  UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
  UPSERT_RIBBON_TRAIL_FLAG_CLOSED,
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
  writeUpsertGroupLayer,
  writeUpsertQuadFieldHeaderWithSemantics,
  writeUpsertQuadFieldItem,
  writeUpsertRibbonTrailHeaderWithSemantics,
  writeUpsertRibbonTrailPoint,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const GROUP_ID: u32 = 7610;
const GLOW_ID: u32 = 7611;
const TRAIL_ID: u32 = 7612;
const FIELD_ID: u32 = 7613;
const TRAIL_POINT_COUNT: u32 = 5;
const FIELD_ITEM_COUNT: u32 = 5;

const GROUP_LAYER_BYTES: u32 = UPSERT_GROUP_LAYER_COMMAND_BYTES;
const GLOW_BYTES: u32 = UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS;
const TRAIL_BYTES: u32 = upsertRibbonTrailCommandBytesWithSemantics(TRAIL_POINT_COUNT);
const FIELD_BYTES: u32 = upsertQuadFieldCommandBytesWithSemantics(FIELD_ITEM_COUNT);
const LEFT_TOTAL_BYTES: u32 = GROUP_LAYER_BYTES + GLOW_BYTES + TRAIL_BYTES + FIELD_BYTES;

function emitGroupLayer(
  outputPtr: usize,
  hasBlendOverride: bool,
  blendMode: u32,
  sortBias: i32,
): void {
  writeUpsertGroupLayer(outputPtr, GROUP_ID, hasBlendOverride, blendMode, sortBias);
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

  if (button == BUTTON_MIDDLE) {
    if (outputCap < GROUP_LAYER_BYTES) {
      return 0;
    }
    emitGroupLayer(outputPtr, true, BLEND_MODE_ADD, 46);
    return GROUP_LAYER_BYTES;
  }

  if (button != BUTTON_LEFT || outputCap < LEFT_TOTAL_BYTES) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));

  emitGroupLayer(outputPtr, false, BLEND_MODE_NORMAL, 0);
  emitGlow(outputPtr + <usize>GROUP_LAYER_BYTES, x, y);
  emitRibbon(outputPtr + <usize>(GROUP_LAYER_BYTES + GLOW_BYTES), x, y);
  emitQuadField(outputPtr + <usize>(GROUP_LAYER_BYTES + GLOW_BYTES + TRAIL_BYTES), x, y, seed);
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
