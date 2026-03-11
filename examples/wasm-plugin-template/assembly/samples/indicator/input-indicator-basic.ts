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
  INDICATOR_POINTER_BUTTON_BYTES,
  INDICATOR_POINTER_SHELL_BYTES,
  INDICATOR_POINTER_WHEEL_BYTES,
  emitIndicatorKeyPanel,
  emitIndicatorPointerButton,
  emitIndicatorPointerShell,
  emitIndicatorPointerWheel,
} from "../../common/input-indicator-style";

const KEY_BYTES: u32 = INDICATOR_KEY_PANEL_BYTES + SPAWN_TEXT_COMMAND_BYTES;
const POINTER_BYTES: u32 =
  INDICATOR_POINTER_SHELL_BYTES +
  INDICATOR_POINTER_BUTTON_BYTES +
  INDICATOR_POINTER_WHEEL_BYTES +
  SPAWN_TEXT_COMMAND_BYTES;
const TOTAL_BYTES: u32 = POINTER_BYTES > KEY_BYTES ? POINTER_BYTES : KEY_BYTES;

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

function clickTint(button: u8): u32 {
  if (button == BUTTON_RIGHT) return 0xFFFF7B72;
  if (button == BUTTON_MIDDLE) return 0xFFFFD166;
  if (button == BUTTON_LEFT) return 0xFF44E6FF;
  return 0xFFE8EDF2;
}

function scrollTint(delta: i32, horizontal: bool): u32 {
  if (horizontal) {
    return delta >= 0 ? 0xFFFFB347 : 0xFFFF8A65;
  }
  return delta >= 0 ? 0xFF57F2B4 : 0xFF7F8CFF;
}

function keyTint(modifierMask: u8, detailFlags: u8, primaryCode: u32): u32 {
  if ((detailFlags & INDICATOR_DETAIL_FLAG_KEY_SYSTEM) != 0) {
    return 0xFFFFA0C8;
  }
  if ((modifierMask & INDICATOR_MODIFIER_META) != 0) {
    return 0xFF77D6FF;
  }
  if ((modifierMask & INDICATOR_MODIFIER_CTRL) != 0) {
    return 0xFF93F7A7;
  }
  if ((modifierMask & INDICATOR_MODIFIER_ALT) != 0) {
    return 0xFFFFD27A;
  }
  if ((modifierMask & INDICATOR_MODIFIER_SHIFT) != 0) {
    return 0xFFC8B6FF;
  }
  if (primaryCode == 0x0D || primaryCode == 0x20) {
    return 0xFFBDE7FF;
  }
  return 0xFFEAF4FF;
}

function panelWidthForEvent(
  kind: u8,
  sizePx: f32,
  streak: u16,
  modifierMask: u8,
  horizontal: bool,
): f32 {
  const streakBoost = <f32>(streak > 1 ? streak - 1 : 0) * sizePx * <f32>0.05;
  if (kind == EVENT_KIND_INDICATOR_KEY) {
    const modifierCount = <f32>countModifierBits(modifierMask);
    return maxF32(sizePx * <f32>0.86, sizePx * <f32>0.62 + modifierCount * sizePx * <f32>0.12 + streakBoost);
  }
  if (kind == EVENT_KIND_INDICATOR_SCROLL && horizontal) {
    return sizePx * <f32>0.78 + streakBoost;
  }
  return sizePx * <f32>0.48 + streakBoost;
}

function panelHeightForEvent(kind: u8, sizePx: f32, horizontal: bool): f32 {
  if (kind == EVENT_KIND_INDICATOR_KEY) {
    return sizePx * <f32>0.56;
  }
  if (kind == EVENT_KIND_INDICATOR_SCROLL && horizontal) {
    return sizePx * <f32>0.34;
  }
  return sizePx * <f32>0.72;
}

function scrollLabelScale(streak: u16): f32 {
  if (streak >= 100) return <f32>0.52;
  if (streak >= 10) return <f32>0.58;
  return <f32>0.64;
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
  let sizePx: f32 = 96.0;
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

  const horizontal = (readEventFlags(inputPtr) & EVENT_FLAG_SCROLL_HORIZONTAL) != 0;
  let tint: u32 = 0xFF44E6FF;
  if (kind == EVENT_KIND_INDICATOR_CLICK) {
    tint = clickTint(readEventButton(inputPtr));
  } else if (kind == EVENT_KIND_INDICATOR_SCROLL) {
    tint = scrollTint(readEventDelta(inputPtr), horizontal);
  } else {
    tint = keyTint(modifierMask, detailFlags, primaryCode);
  }

  const panelWidth = panelWidthForEvent(kind, sizePx, streak, modifierMask, horizontal);
  const panelHeight = panelHeightForEvent(kind, sizePx, horizontal);
  let textX = x;
  let textY = y;
  let textScale: f32 = kind == EVENT_KIND_INDICATOR_KEY ? <f32>0.98 : <f32>0.90;
  let textOffset: usize = 0;
  let writtenBytes: u32 = 0;

  if (kind == EVENT_KIND_INDICATOR_KEY) {
    emitIndicatorKeyPanel(
      outputPtr,
      x,
      y,
      panelWidth,
      panelHeight,
      tint,
      durationMs,
    );
    textOffset = <usize>INDICATOR_KEY_PANEL_BYTES;
    writtenBytes = KEY_BYTES;
  } else {
    emitIndicatorPointerShell(
      outputPtr,
      x,
      y,
      sizePx,
      tint,
      durationMs,
    );
    let buttonMode: u8 = 0;
    let wheelMode: u8 = 0;
    if (kind == EVENT_KIND_INDICATOR_CLICK) {
      const button = readEventButton(inputPtr);
      buttonMode = button == BUTTON_RIGHT ? 1 : (button == BUTTON_MIDDLE ? 2 : 0);
    } else {
      buttonMode = 3;
      wheelMode = horizontal ? 3 : (readEventDelta(inputPtr) >= 0 ? 1 : 2);
    }
    emitIndicatorPointerButton(
      outputPtr + <usize>INDICATOR_POINTER_SHELL_BYTES,
      x,
      y,
      sizePx,
      buttonMode,
      tint,
      durationMs,
    );
    emitIndicatorPointerWheel(
      outputPtr + <usize>(INDICATOR_POINTER_SHELL_BYTES + INDICATOR_POINTER_BUTTON_BYTES),
      x,
      y,
      sizePx,
      wheelMode,
      tint,
      durationMs,
    );
    if (kind == EVENT_KIND_INDICATOR_CLICK || kind == EVENT_KIND_INDICATOR_SCROLL) {
      // Keep pointer labels in one shared anchor zone for consistent click/scroll feedback.
      textY = y + sizePx * <f32>0.24;
      textScale = kind == EVENT_KIND_INDICATOR_SCROLL
        ? scrollLabelScale(streak)
        : <f32>0.64;
    }
    textOffset = <usize>(
      INDICATOR_POINTER_SHELL_BYTES +
      INDICATOR_POINTER_BUTTON_BYTES +
      INDICATOR_POINTER_WHEEL_BYTES);
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
    tint,
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
