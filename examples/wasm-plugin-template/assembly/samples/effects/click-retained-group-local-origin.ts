import {
  API_VERSION,
  BLEND_MODE_ADD,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  PARTICLE_EMITTER_EMISSION_MODE_RADIAL,
  PARTICLE_EMITTER_SPAWN_SHAPE_RING,
  PARTICLE_EMITTER_STYLE_SOFT_GLOW,
  REMOVE_GROUP_COMMAND_BYTES,
  UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS,
  UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
  UPSERT_GLOW_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN,
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS,
  UPSERT_PARTICLE_EMITTER_FLAG_SCREEN_BLEND,
  UPSERT_PARTICLE_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN,
  UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS,
  UPSERT_SPRITE_EMITTER_FLAG_SCREEN_BLEND,
  UPSERT_SPRITE_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN,
  UPSERT_GROUP_LOCAL_ORIGIN_COMMAND_BYTES,
  UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL_PIVOT_AND_SCALE2D,
  UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
  UPSERT_QUAD_FIELD_FLAG_USE_GROUP_LOCAL_ORIGIN,
  UPSERT_RIBBON_TRAIL_FLAG_CLOSED,
  UPSERT_RIBBON_TRAIL_FLAG_USE_GROUP_LOCAL_ORIGIN,
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
  writeUpsertGroupLocalOrigin,
  writeUpsertGroupTransformWithTailPivotAndScale2D,
  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics,
  writeUpsertQuadFieldHeaderWithSemantics,
  writeUpsertQuadFieldItem,
  writeUpsertRibbonTrailHeaderWithSemantics,
  writeUpsertRibbonTrailPoint,
  writeUpsertSpriteEmitterWithSemantics,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const GROUP_ID: u32 = 7840;
const GLOW_ID: u32 = 7841;
const SPRITE_ID: u32 = 7842;
const PARTICLE_ID: u32 = 7843;
const TRAIL_ID: u32 = 7844;
const FIELD_ID: u32 = 7845;
const TRAIL_POINT_COUNT: u32 = 5;
const FIELD_ITEM_COUNT: u32 = 6;

const LOCAL_ORIGIN_BYTES: u32 = UPSERT_GROUP_LOCAL_ORIGIN_COMMAND_BYTES;
const TRANSFORM_BYTES: u32 = UPSERT_GROUP_TRANSFORM_COMMAND_BYTES_WITH_TAIL_PIVOT_AND_SCALE2D;
const GLOW_BYTES: u32 = UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS;
const SPRITE_BYTES: u32 = UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS;
const PARTICLE_BYTES: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS;
const TRAIL_BYTES: u32 = upsertRibbonTrailCommandBytesWithSemantics(TRAIL_POINT_COUNT);
const FIELD_BYTES: u32 = upsertQuadFieldCommandBytesWithSemantics(FIELD_ITEM_COUNT);
const LEFT_TOTAL_BYTES: u32 =
  LOCAL_ORIGIN_BYTES + TRANSFORM_BYTES + GLOW_BYTES + SPRITE_BYTES + PARTICLE_BYTES + TRAIL_BYTES + FIELD_BYTES;

function emitGroupLocalOrigin(outputPtr: usize, originXPx: f32, originYPx: f32): void {
  writeUpsertGroupLocalOrigin(outputPtr, GROUP_ID, originXPx, originYPx);
}

function emitGroupTransform(
  outputPtr: usize,
  offsetXPx: f32,
  offsetYPx: f32,
  rotationRad: f32,
  uniformScale: f32,
  pivotXPx: f32,
  pivotYPx: f32,
  scaleX: f32,
  scaleY: f32,
): void {
  writeUpsertGroupTransformWithTailPivotAndScale2D(
    outputPtr,
    GROUP_ID,
    offsetXPx,
    offsetYPx,
    rotationRad,
    uniformScale,
    pivotXPx,
    pivotYPx,
    scaleX,
    scaleY,
  );
}

function emitGlow(outputPtr: usize): void {
  writeUpsertGlowEmitterWithSemantics(
    outputPtr,
    <f32>0.0,
    <f32>0.0,
    <f32>112.0,
    <f32>-1.5707963,
    <f32>1.04,
    <f32>88.0,
    <f32>176.0,
    <f32>3.0,
    <f32>9.0,
    <f32>0.18,
    <f32>0.86,
    0xFF74F4FF,
    <f32>0.0,
    <f32>148.0,
    GLOW_ID,
    1380,
    920,
    104,
    UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND | UPSERT_GLOW_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN,
    BLEND_MODE_ADD,
    18,
    GROUP_ID,
  );
}

function emitSprite(outputPtr: usize): void {
  writeUpsertSpriteEmitterWithSemantics(
    outputPtr,
    <f32>54.0,
    <f32>-12.0,
    <f32>84.0,
    <f32>-1.0471976,
    <f32>1.134464,
    <f32>92.0,
    <f32>214.0,
    <f32>18.0,
    <f32>46.0,
    <f32>0.24,
    <f32>0.88,
    0xE8FFF2BA,
    <f32>-0.38,
    <f32>0.58,
    <f32>0.0,
    <f32>162.0,
    SPRITE_ID,
    1220,
    920,
    1,
    108,
    UPSERT_SPRITE_EMITTER_FLAG_SCREEN_BLEND | UPSERT_SPRITE_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN,
    BLEND_MODE_SCREEN,
    20,
    GROUP_ID,
  );
}

function emitParticle(outputPtr: usize): void {
  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(
    outputPtr,
    <f32>-52.0,
    <f32>18.0,
    <f32>92.0,
    <f32>-1.5707963,
    <f32>6.2831853,
    <f32>96.0,
    <f32>224.0,
    <f32>2.6,
    <f32>7.8,
    <f32>0.20,
    <f32>0.82,
    0xFF7AF8FF,
    <f32>0.0,
    <f32>144.0,
    PARTICLE_ID,
    1280,
    940,
    144,
    PARTICLE_EMITTER_STYLE_SOFT_GLOW,
    UPSERT_PARTICLE_EMITTER_FLAG_SCREEN_BLEND | UPSERT_PARTICLE_EMITTER_FLAG_USE_GROUP_LOCAL_ORIGIN,
    PARTICLE_EMITTER_EMISSION_MODE_RADIAL,
    PARTICLE_EMITTER_SPAWN_SHAPE_RING,
    <f32>18.0,
    <f32>18.0,
    <f32>0.68,
    <f32>1.35,
    <f32>520.0,
    <f32>2.0,
    <f32>1.4,
    <f32>0.64,
    <f32>1.54,
    <f32>0.36,
    <f32>0.0,
    0x44FFFFFF,
    0xFF4FE6FF,
    BLEND_MODE_SCREEN,
    21,
    GROUP_ID,
  );
}

function emitRibbon(outputPtr: usize): void {
  writeUpsertRibbonTrailHeaderWithSemantics(
    outputPtr,
    TRAIL_POINT_COUNT,
    <f32>0.94,
    <f32>10.0,
    0xFF7AF8FF,
    0x667AF8FF,
    1080,
    TRAIL_ID,
    UPSERT_RIBBON_TRAIL_FLAG_CLOSED | UPSERT_RIBBON_TRAIL_FLAG_USE_GROUP_LOCAL_ORIGIN,
    BLEND_MODE_SCREEN,
    22,
    GROUP_ID,
  );
  for (let index: u32 = 0; index < TRAIL_POINT_COUNT; index += 1) {
    const progress = <f32>index / <f32>(TRAIL_POINT_COUNT - 1);
    const px = (progress - <f32>0.5) * <f32>116.0;
    const py = <f32>-18.0 + <f32>Math.sin(progress * <f32>6.2831853) * <f32>26.0;
    const widthPx = <f32>12.0 + (<f32>1.0 - <f32>Math.abs(progress - <f32>0.5) * <f32>2.0) * <f32>22.0;
    writeUpsertRibbonTrailPoint(outputPtr, index, px, py, widthPx);
  }
}

function emitQuadField(outputPtr: usize, seed: u32): void {
  writeUpsertQuadFieldHeaderWithSemantics(
    outputPtr,
    FIELD_ITEM_COUNT,
    1240,
    FIELD_ID,
    UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND | UPSERT_QUAD_FIELD_FLAG_USE_GROUP_LOCAL_ORIGIN,
    BLEND_MODE_ADD,
    24,
    GROUP_ID,
  );

  for (let index: u32 = 0; index < FIELD_ITEM_COUNT; index += 1) {
    const angle = (<f32>index / <f32>FIELD_ITEM_COUNT) * <f32>6.2831853;
    const radius = <f32>34.0 + <f32>rangedFromSeed(seed, index * 7 + 3, 0, 26);
    const localX = <f32>Math.cos(angle) * radius;
    const localY = <f32>Math.sin(angle) * radius;
    const widthPx = <f32>24.0 + <f32>rangedFromSeed(seed, index * 13 + 5, 0, 24);
    const heightPx = <f32>20.0 + <f32>rangedFromSeed(seed, index * 17 + 9, 0, 16);
    const tile = index % 3;
    writeUpsertQuadFieldItem(
      outputPtr,
      index,
      localX,
      localY,
      widthPx,
      heightPx,
      <f32>0.84,
      angle * <f32>0.5,
      0xFF84F6FF,
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
    if (outputCap < LOCAL_ORIGIN_BYTES) {
      return 0;
    }
    emitGroupLocalOrigin(outputPtr, x, y);
    return LOCAL_ORIGIN_BYTES;
  }

  if (button != BUTTON_LEFT || outputCap < LEFT_TOTAL_BYTES) {
    return 0;
  }

  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  emitGroupLocalOrigin(outputPtr, x, y);
  emitGroupTransform(
    outputPtr + <usize>LOCAL_ORIGIN_BYTES,
    <f32>0.0,
    <f32>0.0,
    <f32>0.42,
    <f32>1.18,
    <f32>18.0,
    <f32>-14.0,
    <f32>1.34,
    <f32>0.72,
  );
  emitGlow(outputPtr + <usize>(LOCAL_ORIGIN_BYTES + TRANSFORM_BYTES));
  emitSprite(outputPtr + <usize>(LOCAL_ORIGIN_BYTES + TRANSFORM_BYTES + GLOW_BYTES));
  emitParticle(outputPtr + <usize>(LOCAL_ORIGIN_BYTES + TRANSFORM_BYTES + GLOW_BYTES + SPRITE_BYTES));
  emitRibbon(outputPtr + <usize>(LOCAL_ORIGIN_BYTES + TRANSFORM_BYTES + GLOW_BYTES + SPRITE_BYTES + PARTICLE_BYTES));
  emitQuadField(
    outputPtr + <usize>(LOCAL_ORIGIN_BYTES + TRANSFORM_BYTES + GLOW_BYTES + SPRITE_BYTES + PARTICLE_BYTES + TRAIL_BYTES),
    seed,
  );
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
