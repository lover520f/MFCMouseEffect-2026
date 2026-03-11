import {
  API_VERSION,
  BLEND_MODE_ADD,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  REMOVE_GROUP_COMMAND_BYTES,
  UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS,
  UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
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
  writeUpsertQuadFieldHeaderWithSemantics,
  writeUpsertQuadFieldItem,
  writeUpsertRibbonTrailHeaderWithSemantics,
  writeUpsertRibbonTrailPoint,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const GROUP_ID: u32 = 6110;
const GLOW_ID: u32 = 6111;
const TRAIL_ID: u32 = 6112;
const FIELD_ID: u32 = 6113;
const TRAIL_POINT_COUNT: u32 = 5;
const FIELD_ITEM_COUNT: u32 = 4;
const UPSERT_GLOW_BYTES: u32 = UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS;
const UPSERT_TRAIL_BYTES: u32 = upsertRibbonTrailCommandBytesWithSemantics(TRAIL_POINT_COUNT);
const UPSERT_FIELD_BYTES: u32 = upsertQuadFieldCommandBytesWithSemantics(FIELD_ITEM_COUNT);
const UPSERT_TOTAL_BYTES: u32 = UPSERT_GLOW_BYTES + UPSERT_TRAIL_BYTES + UPSERT_FIELD_BYTES;

function emitGlow(outputPtr: usize, x: f32, y: f32, button: u8): void {
  const color = button == BUTTON_MIDDLE ? 0xFFFFC56B : 0xFF66F2FF;
  writeUpsertGlowEmitterWithSemantics(
    outputPtr,
    x,
    y,
    button == BUTTON_MIDDLE ? <f32>86.0 : <f32>108.0,
    button == BUTTON_MIDDLE ? <f32>-1.5707963 : <f32>-1.0471976,
    <f32>0.92,
    <f32>84.0,
    <f32>168.0,
    <f32>3.0,
    <f32>8.0,
    <f32>0.16,
    <f32>0.78,
    color,
    <f32>0.0,
    button == BUTTON_MIDDLE ? <f32>96.0 : <f32>148.0,
    GLOW_ID,
    1200,
    860,
    96,
    UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    18,
    GROUP_ID,
  );
}

function emitRibbon(outputPtr: usize, x: f32, y: f32, button: u8): void {
  writeUpsertRibbonTrailHeaderWithSemantics(
    outputPtr,
    TRAIL_POINT_COUNT,
    <f32>0.88,
    <f32>9.0,
    button == BUTTON_MIDDLE ? 0xFFFFBE73 : 0xFF5EF6FF,
    button == BUTTON_MIDDLE ? 0x66FFBE73 : 0x665EF6FF,
    960,
    TRAIL_ID,
    UPSERT_RIBBON_TRAIL_FLAG_CLOSED,
    BLEND_MODE_SCREEN,
    20,
    GROUP_ID,
  );
  for (let index: u32 = 0; index < TRAIL_POINT_COUNT; index += 1) {
    const progress = <f32>index / <f32>(TRAIL_POINT_COUNT - 1);
    const px = x + (progress - <f32>0.5) * <f32>92.0;
    const py = y - <f32>22.0 + <f32>Math.sin(progress * <f32>6.2831853) * (button == BUTTON_MIDDLE ? <f32>18.0 : <f32>26.0);
    const widthPx = <f32>(<f32>14.0 + (<f32>1.0 - <f32>Math.abs(progress - <f32>0.5) * <f32>2.0) * <f32>18.0);
    writeUpsertRibbonTrailPoint(outputPtr, index, px, py, widthPx);
  }
}

function emitQuadField(outputPtr: usize, x: f32, y: f32, seed: u32, button: u8): void {
  writeUpsertQuadFieldHeaderWithSemantics(
    outputPtr,
    FIELD_ITEM_COUNT,
    1100,
    FIELD_ID,
    UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    22,
    GROUP_ID,
  );

  for (let index: u32 = 0; index < FIELD_ITEM_COUNT; index += 1) {
    const angle = (<f32>index / <f32>FIELD_ITEM_COUNT) * <f32>6.2831853;
    const radius = <f32>34.0 + <f32>rangedFromSeed(seed, index * 11 + 3, 0, 20);
    const localX = x + <f32>Math.cos(angle) * radius;
    const localY = y + <f32>Math.sin(angle) * radius;
    const widthPx = <f32>28.0 + <f32>rangedFromSeed(seed, index * 17 + 5, 0, 24);
    const heightPx = <f32>24.0 + <f32>rangedFromSeed(seed, index * 19 + 7, 0, 18);
    const tile = index % 3;
    writeUpsertQuadFieldItem(
      outputPtr,
      index,
      localX,
      localY,
      widthPx,
      heightPx,
      <f32>0.82,
      angle * <f32>0.55,
      button == BUTTON_MIDDLE ? 0xFFFFC86B : 0xFF74EFFF,
      tile,
      tile == 0 ? <f32>0.0 : tile == 1 ? <f32>0.5 : <f32>0.0,
      tile == 2 ? <f32>0.5 : <f32>0.0,
      tile == 0 ? <f32>1.0 : tile == 1 ? <f32>1.0 : <f32>0.5,
      tile == 2 ? <f32>1.0 : <f32>0.5,
      <f32>signedFromSeed(seed, index * 23 + 11, 16),
      <f32>signedFromSeed(seed, index * 29 + 13, 16) - <f32>14.0,
      <f32>signedFromSeed(seed, index * 31 + 17, 6),
      button == BUTTON_MIDDLE ? <f32>10.0 : <f32>18.0,
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

  if ((button != BUTTON_LEFT && button != BUTTON_MIDDLE) || outputCap < UPSERT_TOTAL_BYTES) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  emitGlow(outputPtr, x, y, button);
  emitRibbon(outputPtr + <usize>UPSERT_GLOW_BYTES, x, y, button);
  emitQuadField(outputPtr + <usize>(UPSERT_GLOW_BYTES + UPSERT_TRAIL_BYTES), x, y, seed, button);
  return UPSERT_TOTAL_BYTES;
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
