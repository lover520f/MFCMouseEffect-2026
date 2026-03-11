import {
  API_VERSION,
  BLEND_MODE_SCREEN,
  BUTTON_RIGHT,
  EVENT_INPUT_BYTES,
  EVENT_KIND_CLICK,
  REMOVE_SPRITE_EMITTER_COMMAND_BYTES,
  UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP,
  UPSERT_SPRITE_EMITTER_FLAG_SCREEN_BLEND,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  writeRemoveSpriteEmitter,
  writeUpsertSpriteEmitterWithSemanticsAndClip,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs } from "../../common/random";

const EMITTER_ID: u32 = 1;
const EMITTER_GROUP_ID: u32 = 1017;

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
    if (outputCap < REMOVE_SPRITE_EMITTER_COMMAND_BYTES) {
      return 0;
    }
    writeRemoveSpriteEmitter(outputPtr, EMITTER_ID);
    return REMOVE_SPRITE_EMITTER_COMMAND_BYTES;
  }

  if (outputCap < UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP) {
    return 0;
  }

  const seed = seedFromTickMs(readEventTickMs(inputPtr));
  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const directionDeg = <f32>rangedFromSeed(seed, 2, -42, 42);
  const directionRad = directionDeg * 0.017453292;
  const spreadRad: f32 = 1.2217305;
  const ttlMs: u32 = 900 + <u32>rangedFromSeed(seed, 5, 0, 220);
  const particleLifeMs: u32 = 760 + <u32>rangedFromSeed(seed, 8, 0, 220);
  const tintArgb: u32 = 0xE8FFFFFF;
  const imageId: u32 = seed % 3;

  writeUpsertSpriteEmitterWithSemanticsAndClip(
    outputPtr,
    x,
    y,
    72.0 + <f32>rangedFromSeed(seed, 11, 0, 18),
    directionRad,
    spreadRad,
    96.0,
    210.0 + <f32>rangedFromSeed(seed, 14, 0, 54),
    18.0,
    54.0,
    0.24,
    0.92,
    tintArgb,
    -0.55,
    0.55,
    0.0,
    180.0,
    EMITTER_ID,
    ttlMs,
    particleLifeMs,
    imageId,
    132,
    UPSERT_SPRITE_EMITTER_FLAG_SCREEN_BLEND,
    BLEND_MODE_SCREEN,
    28,
    EMITTER_GROUP_ID,
    x - 60.0,
    y - 52.0,
    120.0,
    104.0,
  );
  return UPSERT_SPRITE_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP;
}

export function mfx_plugin_on_frame(
  _inputPtr: usize,
  _inputLen: u32,
  _outputPtr: usize,
  _outputCap: u32,
): u32 {
  return 0;
}
