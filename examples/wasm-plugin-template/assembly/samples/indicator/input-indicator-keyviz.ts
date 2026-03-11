import {
  API_VERSION,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  EVENT_FLAG_SCROLL_HORIZONTAL,
  EVENT_KIND_INDICATOR_CLICK,
  EVENT_KIND_INDICATOR_KEY,
  EVENT_KIND_INDICATOR_SCROLL,
  EVENT_TEXT_ID_INPUT_LABEL,
  INDICATOR_DETAIL_FLAG_KEY_SYSTEM,
  INDICATOR_MODIFIER_ALT,
  INDICATOR_MODIFIER_CTRL,
  INDICATOR_MODIFIER_META,
  INDICATOR_MODIFIER_SHIFT,
  SPAWN_TEXT_COMMAND_BYTES,
  canHandleEvent,
  hasIndicatorContextTail,
  hasIndicatorTail,
  readEventButton,
  readEventDelta,
  readEventFlags,
  readEventKind,
  readEventX,
  readEventY,
  readIndicatorDetailFlags,
  readIndicatorDurationMs,
  readIndicatorModifierMask,
  readIndicatorPrimaryCode,
  readIndicatorSizePx,
  readIndicatorStreak,
  writeSpawnText,
} from "../../common/abi";
import {
  INDICATOR_KEY_PANEL_BYTES,
  emitRoundedRectFill,
} from "../../common/input-indicator-style";

const SHAPE_BYTES: u32 = INDICATOR_KEY_PANEL_BYTES;
const KEY_BYTES: u32 = SHAPE_BYTES * 3 + SPAWN_TEXT_COMMAND_BYTES;
const POINTER_BYTES: u32 = SHAPE_BYTES * 4 + SPAWN_TEXT_COMMAND_BYTES;
const TOTAL_BYTES: u32 = KEY_BYTES > POINTER_BYTES ? KEY_BYTES : POINTER_BYTES;

function maxF32(a: f32, b: f32): f32 {
  return a > b ? a : b;
}

function countModifierBits(mask: u8): u32 {
  let count: u32 = 0;
  if ((mask & INDICATOR_MODIFIER_CTRL) != 0) count += 1;
  if ((mask & INDICATOR_MODIFIER_SHIFT) != 0) count += 1;
  if ((mask & INDICATOR_MODIFIER_ALT) != 0) count += 1;
  if ((mask & INDICATOR_MODIFIER_META) != 0) count += 1;
  return count;
}

function keyTint(modifierMask: u8, detailFlags: u8, primaryCode: u32): u32 {
  if ((detailFlags & INDICATOR_DETAIL_FLAG_KEY_SYSTEM) != 0) {
    return 0xFFFF8FB8;
  }
  if ((modifierMask & INDICATOR_MODIFIER_META) != 0) {
    return 0xFF55C8FF;
  }
  if ((modifierMask & INDICATOR_MODIFIER_CTRL) != 0) {
    return 0xFF7DE6A1;
  }
  if ((modifierMask & INDICATOR_MODIFIER_ALT) != 0) {
    return 0xFFFFC878;
  }
  if ((modifierMask & INDICATOR_MODIFIER_SHIFT) != 0) {
    return 0xFFC6AEFF;
  }
  if (primaryCode == 0x0D || primaryCode == 0x20) {
    return 0xFF9ED8FF;
  }
  return 0xFFE6EEF7;
}

function pointerTint(kind: u8, button: u8, delta: i32, horizontal: bool): u32 {
  if (kind == EVENT_KIND_INDICATOR_CLICK) {
    if (button == BUTTON_RIGHT) return 0xFFFF8F86;
    if (button == BUTTON_MIDDLE) return 0xFFFFD48B;
    return 0xFF82F7C9;
  }
  if (horizontal) {
    return delta >= 0 ? 0xFFFFCB80 : 0xFFFFA266;
  }
  return delta >= 0 ? 0xFF67E8F9 : 0xFF8EA6FF;
}

function emitKeycapLikePanel(
  outputPtr: usize,
  x: f32,
  y: f32,
  width: f32,
  height: f32,
  tint: u32,
  durationMs: u32,
): void {
  const radius = height * 0.22;
  const topLeft = x - width * 0.5;
  const topTop = y - height * 0.5;
  const shadowOffsetY = height * 0.10;

  emitRoundedRectFill(
    outputPtr,
    topLeft,
    topTop + shadowOffsetY,
    width,
    height,
    radius,
    0.86,
    height * 0.05,
    0xB40F1115,
    0x66000000,
    0,
    durationMs,
    4,
  );

  emitRoundedRectFill(
    outputPtr + <usize>SHAPE_BYTES,
    topLeft,
    topTop,
    width,
    height,
    radius,
    0.98,
    height * 0.04,
    0xFFF2F4F8,
    0x44FFFFFF,
    0,
    durationMs,
    8,
  );

  emitRoundedRectFill(
    outputPtr + <usize>(SHAPE_BYTES * 2),
    topLeft + width * 0.06,
    topTop + height * 0.09,
    width * 0.88,
    height * 0.18,
    height * 0.09,
    0.88,
    height * 0.03,
    tint & 0x44FFFFFF,
    tint & 0x60FFFFFF,
    0,
    durationMs,
    12,
  );
}

function emitPointerPanel(
  outputPtr: usize,
  x: f32,
  y: f32,
  sizePx: f32,
  tint: u32,
  button: u8,
  scrollDelta: i32,
  scrollHorizontal: bool,
  durationMs: u32,
): void {
  const width = sizePx * 0.60;
  const height = sizePx * 0.82;
  const radius = width * 0.26;
  const left = x - width * 0.5;
  const top = y - height * 0.5;

  emitRoundedRectFill(
    outputPtr,
    left,
    top + height * 0.08,
    width,
    height,
    radius,
    0.90,
    sizePx * 0.05,
    0xA40B0D12,
    0x66000000,
    0,
    durationMs,
    5,
  );

  emitRoundedRectFill(
    outputPtr + <usize>SHAPE_BYTES,
    left,
    top,
    width,
    height,
    radius,
    0.92,
    sizePx * 0.05,
    0xE0181B24,
    0x551A1F2A,
    0,
    durationMs,
    10,
  );

  let accentLeft = left + width * 0.10;
  let accentTop = top + height * 0.10;
  let accentWidth = width * 0.34;
  let accentHeight = height * 0.34;
  if (button == BUTTON_RIGHT) {
    accentLeft = left + width * 0.56;
  } else if (button == BUTTON_MIDDLE) {
    accentLeft = left + width * 0.43;
    accentWidth = width * 0.14;
    accentHeight = height * 0.38;
  }

  const isScroll = scrollDelta != 0;
  if (isScroll) {
    accentLeft = left + width * (scrollHorizontal ? 0.20 : 0.43);
    accentTop = top + height * (scrollHorizontal ? 0.35 : (scrollDelta >= 0 ? 0.14 : 0.30));
    accentWidth = width * (scrollHorizontal ? 0.60 : 0.14);
    accentHeight = height * (scrollHorizontal ? 0.12 : 0.16);
  }

  emitRoundedRectFill(
    outputPtr + <usize>(SHAPE_BYTES * 2),
    accentLeft,
    accentTop,
    accentWidth,
    accentHeight,
    maxF32(accentWidth * 0.25, accentHeight * 0.25),
    0.96,
    sizePx * 0.04,
    tint & 0x8EFFFFFF,
    tint & 0x7CFFFFFF,
    0,
    durationMs,
    14,
  );

  emitRoundedRectFill(
    outputPtr + <usize>(SHAPE_BYTES * 3),
    left + width * 0.43,
    top + height * 0.20,
    width * 0.14,
    height * 0.22,
    width * 0.08,
    0.84,
    sizePx * 0.03,
    0xE2E8EEF8,
    0x33FFFFFF,
    0,
    durationMs,
    16,
  );
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
  const kind = readEventKind(inputPtr);
  if (kind != EVENT_KIND_INDICATOR_CLICK &&
      kind != EVENT_KIND_INDICATOR_SCROLL &&
      kind != EVENT_KIND_INDICATOR_KEY) {
    return 0;
  }
  if (!canHandleEvent(inputLen, outputCap, TOTAL_BYTES)) {
    return 0;
  }

  const x = <f32>readEventX(inputPtr);
  const y = <f32>readEventY(inputPtr);
  let sizePx: f32 = 92.0;
  let durationMs: u32 = 420;
  if (hasIndicatorTail(inputLen)) {
    sizePx = <f32>readIndicatorSizePx(inputPtr);
    durationMs = <u32>readIndicatorDurationMs(inputPtr);
  }

  let primaryCode: u32 = 0;
  let streak: u16 = 1;
  let modifierMask: u8 = 0;
  let detailFlags: u8 = 0;
  if (hasIndicatorContextTail(inputLen)) {
    primaryCode = readIndicatorPrimaryCode(inputPtr);
    streak = readIndicatorStreak(inputPtr);
    modifierMask = readIndicatorModifierMask(inputPtr);
    detailFlags = readIndicatorDetailFlags(inputPtr);
  }

  let tint: u32 = 0xFFE6EEF7;
  const button = readEventButton(inputPtr);
  const delta = readEventDelta(inputPtr);
  const horizontal = (readEventFlags(inputPtr) & EVENT_FLAG_SCROLL_HORIZONTAL) != 0;
  if (kind == EVENT_KIND_INDICATOR_KEY) {
    tint = keyTint(modifierMask, detailFlags, primaryCode);
  } else {
    tint = pointerTint(kind, button, delta, horizontal);
  }

  let textX = x;
  let textY = y;
  let writtenBytes: u32 = 0;
  let textScale = <f32>0.88;
  let textColor = 0xFF10151D;
  let textOffset: usize = 0;

  if (kind == EVENT_KIND_INDICATOR_KEY) {
    const modifierCount = <f32>countModifierBits(modifierMask);
    const streakBoost = <f32>(streak > 1 ? streak - 1 : 0) * sizePx * <f32>0.05;
    const width = maxF32(sizePx * <f32>0.84, sizePx * (<f32>0.56 + modifierCount * <f32>0.12) + streakBoost);
    const height = sizePx * <f32>0.58;
    emitKeycapLikePanel(outputPtr, x, y, width, height, tint, durationMs);
    textY = y + height * <f32>0.14;
    textOffset = <usize>(SHAPE_BYTES * 3);
    writtenBytes = KEY_BYTES;
  } else {
    emitPointerPanel(outputPtr, x, y, sizePx, tint, button, delta, horizontal, durationMs);
    textY = y + sizePx * <f32>0.58;
    textScale = <f32>0.78;
    textColor = 0xFFEFF4FB;
    textOffset = <usize>(SHAPE_BYTES * 4);
    writtenBytes = POINTER_BYTES;
  }

  writeSpawnText(
    outputPtr + textOffset,
    textX,
    textY,
    0.0,
    0.0,
    0.0,
    0.0,
    textScale,
    0.0,
    1.0,
    textColor,
    0,
    durationMs,
    EVENT_TEXT_ID_INPUT_LABEL,
  );

  return writtenBytes;
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
