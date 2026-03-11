import {
  API_VERSION,
  BLEND_MODE_SCREEN,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_INPUT_BYTES,
  EVENT_KIND_CLICK,
  REMOVE_GLOW_EMITTER_COMMAND_BYTES,
  UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP,
  UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
  readEventButton,
  readEventKind,
  readEventTickMs,
  readEventX,
  readEventY,
  writeRemoveGlowEmitter,
  writeUpsertGlowEmitterWithSemanticsAndClip,
} from "../../common/abi";
import { rangedFromSeed, seedFromTickMs } from "../../common/random";

const EMITTER_ID: u32 = 1;
const EMITTER_GROUP_ID: u32 = 1003;

function colorForButton(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFF7A00;
  if (button == BUTTON_MIDDLE) return 0xFF9B7BFF;
  return 0xFF33E8FF;
}

function directionForSeed(seed: u32): f32 {
  const raw = <f32>rangedFromSeed(seed, 2, -60, 60);
  return raw * 0.017453292;
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
    if (outputCap < REMOVE_GLOW_EMITTER_COMMAND_BYTES) {
      return 0;
    }
    writeRemoveGlowEmitter(outputPtr, EMITTER_ID);
    return REMOVE_GLOW_EMITTER_COMMAND_BYTES;
  }

  if (outputCap < UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP) {
    return 0;
  }

  const tickSeed = seedFromTickMs(readEventTickMs(inputPtr));
  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  const color = colorForButton(button);
  const directionRad = directionForSeed(tickSeed);
  const spreadRad: f32 = 1.3962634;
  const ttlMs: u32 = 720 + <u32>rangedFromSeed(tickSeed, 6, 0, 180);
  const particleLifeMs: u32 = 680 + <u32>rangedFromSeed(tickSeed, 9, 0, 180);

  writeUpsertGlowEmitterWithSemanticsAndClip(
    outputPtr,
    x,
    y,
    92.0 + <f32>rangedFromSeed(tickSeed, 12, 0, 24),
    directionRad,
    spreadRad,
    120.0,
    250.0 + <f32>rangedFromSeed(tickSeed, 15, 0, 60),
    2.2,
    7.4,
    0.18,
    0.84,
    color,
    0.0,
    180.0,
    EMITTER_ID,
    ttlMs,
    particleLifeMs,
    160,
    UPSERT_GLOW_EMITTER_FLAG_SCREEN_BLEND,
    BLEND_MODE_SCREEN,
    24,
    EMITTER_GROUP_ID,
    x - 56.0,
    y - 44.0,
    112.0,
    88.0,
  );
  return UPSERT_GLOW_EMITTER_COMMAND_BYTES_WITH_SEMANTICS_AND_CLIP;
}

export function mfx_plugin_on_frame(
  inputPtr: usize,
  inputLen: u32,
  outputPtr: usize,
  outputCap: u32,
): u32 {
  return 0;
}
