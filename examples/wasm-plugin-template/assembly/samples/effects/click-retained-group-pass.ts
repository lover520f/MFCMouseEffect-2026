import {
  API_VERSION,
  BLEND_MODE_ADD,
  BLEND_MODE_SCREEN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_KIND_CLICK,
  GROUP_PASS_KIND_ECHO_LIKE,
  GROUP_PASS_KIND_AFTERIMAGE_LIKE,
  GROUP_PASS_KIND_SOFT_BLOOM_LIKE,
  GROUP_PASS_BLEND_MODE_LERP,
  GROUP_PASS_BLEND_MODE_MULTIPLY,
  GROUP_PASS_MODE_SWIRL,
  GROUP_PASS_TEMPORAL_MODE_LINEAR,
  GROUP_PASS_ROUTE_GLOW,
  GROUP_PASS_TEMPORAL_MODE_PULSE,
  GROUP_PASS_ROUTE_PARTICLE,
  GROUP_PASS_ROUTE_QUAD,
  GROUP_PASS_ROUTE_RIBBON,
  GROUP_PASS_ROUTE_SPRITE,
  PARTICLE_EMITTER_EMISSION_MODE_RADIAL,
  PARTICLE_EMITTER_SPAWN_SHAPE_RING,
  PARTICLE_EMITTER_STYLE_SOFT_GLOW,
  REMOVE_GROUP_COMMAND_BYTES,
  UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS,
  UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
  UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_STACK_TAILS,
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS,
  UPSERT_PARTICLE_EMITTER_FLAG_SCREEN_BLEND,
  UPSERT_QUAD_FIELD_FLAG_SCREEN_BLEND,
  UPSERT_RIBBON_TRAIL_FLAG_CLOSED,
  UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS,
  UPSERT_SPRITE_EMITTER_FLAG_SCREEN_BLEND,
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
  writeUpsertGroupPassWithTertiaryStackTails,
  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics,
  writeUpsertQuadFieldHeaderWithSemantics,
  writeUpsertQuadFieldItem,
  writeUpsertRibbonTrailHeaderWithSemantics,
  writeUpsertRibbonTrailPoint,
  writeUpsertSpriteEmitterWithSemantics,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs, signedFromSeed } from "../../common/random";

const GROUP_ID: u32 = 7940;
const GLOW_ID: u32 = 7941;
const SPRITE_ID: u32 = 7942;
const PARTICLE_ID: u32 = 7943;
const TRAIL_ID: u32 = 7944;
const FIELD_ID: u32 = 7945;
const TRAIL_POINT_COUNT: u32 = 5;
const FIELD_ITEM_COUNT: u32 = 6;

const GLOW_BYTES: u32 = UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS;
const SPRITE_BYTES: u32 = UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS;
const PARTICLE_BYTES: u32 =
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS;
const TRAIL_BYTES: u32 = upsertRibbonTrailCommandBytesWithSemantics(TRAIL_POINT_COUNT);
const FIELD_BYTES: u32 = upsertQuadFieldCommandBytesWithSemantics(FIELD_ITEM_COUNT);
const PASS_BYTES: u32 = UPSERT_GROUP_PASS_COMMAND_BYTES_WITH_TERTIARY_STACK_TAILS;
const LEFT_TOTAL_BYTES: u32 = GLOW_BYTES + SPRITE_BYTES + PARTICLE_BYTES + TRAIL_BYTES + FIELD_BYTES;

function emitGlow(outputPtr: usize, x: f32, y: f32): void {
  writeUpsertGlowEmitterWithSemantics(
    outputPtr,
    x,
    y,
    <f32>118.0,
    <f32>-1.5707963,
    <f32>1.08,
    <f32>92.0,
    <f32>194.0,
    <f32>3.0,
    <f32>9.0,
    <f32>0.20,
    <f32>0.90,
    0xFFFFB86C,
    <f32>0.0,
    <f32>156.0,
    GLOW_ID,
    1400,
    920,
    108,
    UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
    BLEND_MODE_ADD,
    18,
    GROUP_ID,
  );
}

function emitSprite(outputPtr: usize, x: f32, y: f32): void {
  writeUpsertSpriteEmitterWithSemantics(
    outputPtr,
    x + <f32>52.0,
    y - <f32>14.0,
    <f32>86.0,
    <f32>-1.0471976,
    <f32>1.12,
    <f32>90.0,
    <f32>216.0,
    <f32>18.0,
    <f32>48.0,
    <f32>0.22,
    <f32>0.88,
    0xFFF8D8A0,
    <f32>-0.34,
    <f32>0.52,
    <f32>0.0,
    <f32>156.0,
    SPRITE_ID,
    1240,
    920,
    1,
    104,
    UPSERT_SPRITE_EMITTER_FLAG_SCREEN_BLEND,
    BLEND_MODE_SCREEN,
    20,
    GROUP_ID,
  );
}

function emitParticle(outputPtr: usize, x: f32, y: f32): void {
  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(
    outputPtr,
    x - <f32>54.0,
    y + <f32>18.0,
    <f32>92.0,
    <f32>-1.5707963,
    <f32>6.2831853,
    <f32>94.0,
    <f32>220.0,
    <f32>2.8,
    <f32>8.4,
    <f32>0.20,
    <f32>0.84,
    0xFFFFCC7A,
    <f32>0.0,
    <f32>142.0,
    PARTICLE_ID,
    1280,
    940,
    144,
    PARTICLE_EMITTER_STYLE_SOFT_GLOW,
    UPSERT_PARTICLE_EMITTER_FLAG_SCREEN_BLEND,
    PARTICLE_EMITTER_EMISSION_MODE_RADIAL,
    PARTICLE_EMITTER_SPAWN_SHAPE_RING,
    <f32>18.0,
    <f32>18.0,
    <f32>0.66,
    <f32>1.26,
    <f32>420.0,
    <f32>1.8,
    <f32>1.32,
    <f32>0.62,
    <f32>1.46,
    <f32>0.32,
    <f32>0.0,
    0x66FFF1D0,
    0xFFFFB86C,
    BLEND_MODE_SCREEN,
    21,
    GROUP_ID,
  );
}

function emitRibbon(outputPtr: usize, x: f32, y: f32): void {
  writeUpsertRibbonTrailHeaderWithSemantics(
    outputPtr,
    TRAIL_POINT_COUNT,
    <f32>0.94,
    <f32>10.0,
    0xFFFFC77B,
    0x66FFC77B,
    1080,
    TRAIL_ID,
    UPSERT_RIBBON_TRAIL_FLAG_CLOSED,
    BLEND_MODE_SCREEN,
    22,
    GROUP_ID,
  );
  for (let index: u32 = 0; index < TRAIL_POINT_COUNT; index += 1) {
    const progress = <f32>index / <f32>(TRAIL_POINT_COUNT - 1);
    const px = x + (progress - <f32>0.5) * <f32>112.0;
    const py = y - <f32>20.0 + <f32>Math.sin(progress * <f32>6.2831853) * <f32>24.0;
    const widthPx = <f32>12.0 + (<f32>1.0 - <f32>Math.abs(progress - <f32>0.5) * <f32>2.0) * <f32>22.0;
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
    const radius = <f32>32.0 + <f32>rangedFromSeed(seed, index * 7 + 3, 0, 26);
    const localX = x + <f32>Math.cos(angle) * radius;
    const localY = y + <f32>Math.sin(angle) * radius;
    const widthPx = <f32>26.0 + <f32>rangedFromSeed(seed, index * 13 + 5, 0, 24);
    const heightPx = <f32>22.0 + <f32>rangedFromSeed(seed, index * 17 + 9, 0, 16);
    const tile = index % 3;
    writeUpsertQuadFieldItem(
      outputPtr,
      index,
      localX,
      localY,
      widthPx,
      heightPx,
      <f32>0.84,
      angle * <f32>0.48,
      0xFFFFD09A,
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
    if (outputCap < PASS_BYTES) {
      return 0;
    }
    writeUpsertGroupPassWithTertiaryStackTails(
      outputPtr,
      GROUP_ID,
      GROUP_PASS_KIND_ECHO_LIKE,
      <f32>0.82,
      <f32>0.68,
      GROUP_PASS_MODE_SWIRL,
      <f32>0.7853982,
      3,
      <f32>0.42,
      GROUP_PASS_KIND_SOFT_BLOOM_LIKE,
      <f32>0.36,
      <f32>0.54,
      GROUP_PASS_BLEND_MODE_LERP,
      <f32>0.44,
      GROUP_PASS_ROUTE_SPRITE | GROUP_PASS_ROUTE_PARTICLE | GROUP_PASS_ROUTE_RIBBON,
      <f32>0.00,
      <f32>0.42,
      <f32>1.18,
      <f32>0.78,
      <f32>0.00,
      <f32>1.35,
      <f32>0.92,
      <f32>0.28,
      GROUP_PASS_TEMPORAL_MODE_PULSE,
      <f32>0.66,
      GROUP_PASS_KIND_AFTERIMAGE_LIKE,
      <f32>0.22,
      <f32>0.58,
      GROUP_PASS_BLEND_MODE_MULTIPLY,
      <f32>0.36,
      GROUP_PASS_ROUTE_GLOW | GROUP_PASS_ROUTE_QUAD,
      <f32>1.24,
      <f32>0.00,
      <f32>0.00,
      <f32>0.00,
      <f32>0.42,
      <f32>-0.85,
      <f32>0.74,
      <f32>0.22,
      GROUP_PASS_TEMPORAL_MODE_LINEAR,
      <f32>0.48,
      4,
      <f32>0.34,
    );
    return PASS_BYTES;
  }

  if (button != BUTTON_LEFT || outputCap < LEFT_TOTAL_BYTES) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const seed = seedFromTickMs(readEventTickMs(inputPtr));

  emitGlow(outputPtr, x, y);
  emitSprite(outputPtr + <usize>GLOW_BYTES, x, y);
  emitParticle(outputPtr + <usize>(GLOW_BYTES + SPRITE_BYTES), x, y);
  emitRibbon(outputPtr + <usize>(GLOW_BYTES + SPRITE_BYTES + PARTICLE_BYTES), x, y);
  emitQuadField(outputPtr + <usize>(GLOW_BYTES + SPRITE_BYTES + PARTICLE_BYTES + TRAIL_BYTES), x, y, seed);
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
