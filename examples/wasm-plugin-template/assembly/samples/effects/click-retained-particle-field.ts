import {
  API_VERSION,
  BLEND_MODE_SCREEN,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_INPUT_BYTES,
  EVENT_KIND_CLICK,
  PARTICLE_EMITTER_EMISSION_MODE_CONE,
  PARTICLE_EMITTER_EMISSION_MODE_RADIAL,
  PARTICLE_EMITTER_SPAWN_SHAPE_DISC,
  PARTICLE_EMITTER_SPAWN_SHAPE_RING,
  PARTICLE_EMITTER_STYLE_SOFT_GLOW,
  PARTICLE_EMITTER_STYLE_SOLID_DISC,
  REMOVE_PARTICLE_EMITTER_COMMAND_BYTES,
  UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS_AND_CLIP,
  UPSERT_PARTICLE_EMITTER_FLAG_SCREEN_BLEND,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  writeRemoveParticleEmitter,
  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemanticsAndClip,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs } from "../../common/random";

const EMITTER_ID: u32 = 1;
const EMITTER_GROUP_ID: u32 = 1029;

function emitterColor(button: u8): u32 {
  if (button == BUTTON_MIDDLE) return 0xFFFF5A5A;
  return 0xFF49E0FF;
}

function emitterStyle(button: u8): u32 {
  if (button == BUTTON_MIDDLE) return PARTICLE_EMITTER_STYLE_SOLID_DISC;
  return PARTICLE_EMITTER_STYLE_SOFT_GLOW;
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
  if (inputLen < EVENT_INPUT_BYTES || readEventKind(inputPtr) != EVENT_KIND_CLICK) {
    return 0;
  }

  const button = readEventButton(inputPtr);
  if (button == BUTTON_RIGHT) {
    if (outputCap < REMOVE_PARTICLE_EMITTER_COMMAND_BYTES) {
      return 0;
    }
    writeRemoveParticleEmitter(outputPtr, EMITTER_ID);
    return REMOVE_PARTICLE_EMITTER_COMMAND_BYTES;
  }

  if (outputCap < UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS_AND_CLIP) {
    return 0;
  }

  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const directionRad = <f32>rangedFromSeed(seed, 2, -55, 55) * 0.017453292;
  const spreadRad: f32 = 1.3089969;
  const ttlMs: u32 = 760 + <u32>rangedFromSeed(seed, 5, 0, 200);
  const particleLifeMs: u32 = 720 + <u32>rangedFromSeed(seed, 8, 0, 180);
  const style = emitterStyle(button);

  const sizeStartScale: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 1.25 : 0.58;
  const sizeEndScale: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0.42 : 1.68;
  const alphaStartScale: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 1.0 : 0.32;
  const alphaEndScale: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0.18 : 0.0;
  const colorStartArgb: u32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0xFFFFE082 : 0x66FFFFFF;
  const colorEndArgb: u32 = emitterColor(button);
  const emissionMode: u32 =
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC
      ? PARTICLE_EMITTER_EMISSION_MODE_CONE
      : PARTICLE_EMITTER_EMISSION_MODE_RADIAL;
  const spawnShape: u32 =
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC
      ? PARTICLE_EMITTER_SPAWN_SHAPE_DISC
      : PARTICLE_EMITTER_SPAWN_SHAPE_RING;
  const spawnRadiusX: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 12.0 : 20.0;
  const spawnRadiusY: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 10.0 : 20.0;
  const spawnInnerRatio: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0.0 : 0.72;
  const dragPerSecond: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 3.2 : 1.45;
  const turbulenceAccel: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0.0 : 560.0;
  const turbulenceFrequencyHz: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0.0 : 2.2;
  const turbulencePhaseJitter: f32 = style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 1.0 : 1.7;

  writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemanticsAndClip(
    outputPtr,
    x,
    y,
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC
      ? 88.0 + <f32>rangedFromSeed(seed, 11, 0, 18)
      : 96.0 + <f32>rangedFromSeed(seed, 11, 0, 26),
    directionRad,
    spreadRad,
    110.0,
    230.0 + <f32>rangedFromSeed(seed, 14, 0, 44),
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 2.8 : 2.2,
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 9.8 : 7.2,
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0.42 : 0.18,
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 0.92 : 0.84,
    emitterColor(button),
    0.0,
    170.0,
    EMITTER_ID,
    ttlMs,
    particleLifeMs,
    style == PARTICLE_EMITTER_STYLE_SOLID_DISC ? 176 : 160,
    style,
    UPSERT_PARTICLE_EMITTER_FLAG_SCREEN_BLEND,
    emissionMode,
    spawnShape,
    spawnRadiusX,
    spawnRadiusY,
    spawnInnerRatio,
    dragPerSecond,
    turbulenceAccel,
    turbulenceFrequencyHz,
    turbulencePhaseJitter,
    sizeStartScale,
    sizeEndScale,
    alphaStartScale,
    alphaEndScale,
    colorStartArgb,
    colorEndArgb,
    BLEND_MODE_SCREEN,
    32,
    EMITTER_GROUP_ID,
    x - 64.0,
    y - 52.0,
    128.0,
    104.0,
  );
  return UPSERT_PARTICLE_EMITTER_COMMAND_BYTES_WITH_SPAWN_DYNAMICS_AND_LIFE_TAILS_AND_SEMANTICS_AND_CLIP;
}

export function mfx_plugin_on_frame(
  _inputPtr: usize,
  _inputLen: u32,
  _outputPtr: usize,
  _outputCap: u32,
): u32 {
  return 0;
}
