import {
  BLEND_MODE_NORMAL,
  PATH_FILL_RULE_NON_ZERO,
  spawnPathFillCommandBytesWithSemantics,
  writePathStrokeNodeClose,
  writePathStrokeNodeLineTo,
  writePathStrokeNodeMoveTo,
  writePathStrokeNodeQuadTo,
  writeSpawnPathFillHeaderWithSemantics,
} from "./abi";

export const INDICATOR_ROUNDED_RECT_NODE_COUNT: u32 = 10;
export const INDICATOR_KEY_PANEL_BYTES: u32 =
  spawnPathFillCommandBytesWithSemantics(INDICATOR_ROUNDED_RECT_NODE_COUNT);
export const INDICATOR_POINTER_SHELL_BYTES: u32 =
  spawnPathFillCommandBytesWithSemantics(INDICATOR_ROUNDED_RECT_NODE_COUNT);
export const INDICATOR_POINTER_BUTTON_BYTES: u32 =
  spawnPathFillCommandBytesWithSemantics(INDICATOR_ROUNDED_RECT_NODE_COUNT);
export const INDICATOR_POINTER_WHEEL_BYTES: u32 =
  spawnPathFillCommandBytesWithSemantics(INDICATOR_ROUNDED_RECT_NODE_COUNT);

export function emitRoundedRectFill(
  outputPtr: usize,
  left: f32,
  top: f32,
  width: f32,
  height: f32,
  radius: f32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  sortKey: i32,
): void {
  const right: f32 = left + width;
  const bottom: f32 = top + height;
  const halfWidth: f32 = width * 0.5;
  const halfHeight: f32 = height * 0.5;
  let clampedRadius: f32 = radius;
  if (clampedRadius < 0.0) {
    clampedRadius = 0.0;
  }
  if (clampedRadius > halfWidth) {
    clampedRadius = halfWidth;
  }
  if (clampedRadius > halfHeight) {
    clampedRadius = halfHeight;
  }

  writeSpawnPathFillHeaderWithSemantics(
    outputPtr,
    INDICATOR_ROUNDED_RECT_NODE_COUNT,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    delayMs,
    lifeMs,
    PATH_FILL_RULE_NON_ZERO,
    BLEND_MODE_NORMAL,
    sortKey,
    0,
  );

  writePathStrokeNodeMoveTo(outputPtr, 0, left + clampedRadius, top);
  writePathStrokeNodeLineTo(outputPtr, 1, right - clampedRadius, top);
  writePathStrokeNodeQuadTo(outputPtr, 2, right, top, right, top + clampedRadius);
  writePathStrokeNodeLineTo(outputPtr, 3, right, bottom - clampedRadius);
  writePathStrokeNodeQuadTo(outputPtr, 4, right, bottom, right - clampedRadius, bottom);
  writePathStrokeNodeLineTo(outputPtr, 5, left + clampedRadius, bottom);
  writePathStrokeNodeQuadTo(outputPtr, 6, left, bottom, left, bottom - clampedRadius);
  writePathStrokeNodeLineTo(outputPtr, 7, left, top + clampedRadius);
  writePathStrokeNodeQuadTo(outputPtr, 8, left, top, left + clampedRadius, top);
  writePathStrokeNodeClose(outputPtr, 9);
}

export function emitIndicatorKeyPanel(
  outputPtr: usize,
  x: f32,
  y: f32,
  width: f32,
  height: f32,
  tint: u32,
  durationMs: u32,
): void {
  emitRoundedRectFill(
    outputPtr,
    x - width * 0.5,
    y - height * 0.5,
    width,
    height,
    height * 0.24,
    0.94,
    height * 0.16,
    tint & 0x24FFFFFF,
    tint & 0x55FFFFFF,
    0,
    durationMs,
    6,
  );
}

export function emitIndicatorPointerShell(
  outputPtr: usize,
  x: f32,
  y: f32,
  sizePx: f32,
  tint: u32,
  durationMs: u32,
): void {
  const width = sizePx * 0.62;
  const height = sizePx * 0.82;
  emitRoundedRectFill(
    outputPtr,
    x - width * 0.5,
    y - height * 0.5,
    width,
    height,
    width * 0.28,
    0.88,
    sizePx * 0.06,
    tint & 0x1BFFFFFF,
    tint & 0x3FFFFFFF,
    0,
    durationMs,
    10,
  );
}

export function emitIndicatorPointerButton(
  outputPtr: usize,
  x: f32,
  y: f32,
  sizePx: f32,
  mode: u8,
  tint: u32,
  durationMs: u32,
): void {
  const bodyWidth = sizePx * 0.62;
  const bodyHeight = sizePx * 0.82;
  const left = x - bodyWidth * 0.5;
  const top = y - bodyHeight * 0.5;

  let accentLeft = left + bodyWidth * 0.12;
  let accentTop = top + bodyHeight * 0.10;
  let accentWidth = bodyWidth * 0.30;
  let accentHeight = bodyHeight * 0.28;
  let radius = accentWidth * 0.28;

  if (mode == 3) {
    emitRoundedRectFill(
      outputPtr,
      accentLeft,
      accentTop,
      accentWidth,
      accentHeight,
      radius,
      0.0,
      0.0,
      0x00000000,
      0x00000000,
      0,
      durationMs,
      12,
    );
    return;
  }

  if (mode == 1) {
    accentLeft = left + bodyWidth * 0.58;
  } else if (mode == 2) {
    accentLeft = left + bodyWidth * 0.43;
    accentTop = top + bodyHeight * 0.14;
    accentWidth = bodyWidth * 0.14;
    accentHeight = bodyHeight * 0.34;
    radius = accentWidth * 0.5;
  }

  emitRoundedRectFill(
    outputPtr,
    accentLeft,
    accentTop,
    accentWidth,
    accentHeight,
    radius,
    0.92,
    sizePx * 0.04,
    tint & 0x44FFFFFF,
    tint & 0x72FFFFFF,
    14,
    durationMs > 60 ? durationMs - 60 : durationMs,
    12,
  );
}

export function emitIndicatorPointerWheel(
  outputPtr: usize,
  x: f32,
  y: f32,
  sizePx: f32,
  mode: u8,
  tint: u32,
  durationMs: u32,
): void {
  const bodyWidth = sizePx * 0.62;
  const bodyHeight = sizePx * 0.82;
  const left = x - bodyWidth * 0.5;
  const top = y - bodyHeight * 0.5;

  let wheelLeft = left + bodyWidth * 0.44;
  let wheelTop = top + bodyHeight * 0.18;
  let wheelWidth = bodyWidth * 0.12;
  let wheelHeight = bodyHeight * 0.22;
  let radius = wheelWidth * 0.5;

  if (mode == 1) {
    wheelTop = top + bodyHeight * 0.13;
    wheelHeight = bodyHeight * 0.14;
    radius = wheelHeight * 0.5;
  } else if (mode == 2) {
    wheelTop = top + bodyHeight * 0.31;
    wheelHeight = bodyHeight * 0.14;
    radius = wheelHeight * 0.5;
  } else if (mode == 3) {
    wheelLeft = left + bodyWidth * 0.24;
    wheelTop = top + bodyHeight * 0.34;
    wheelWidth = bodyWidth * 0.52;
    wheelHeight = bodyHeight * 0.10;
    radius = wheelHeight * 0.5;
  }

  emitRoundedRectFill(
    outputPtr,
    wheelLeft,
    wheelTop,
    wheelWidth,
    wheelHeight,
    radius,
    0.95,
    sizePx * 0.04,
    tint & 0x4CFFFFFF,
    tint & 0x7AFFFFFF,
    10,
    durationMs > 50 ? durationMs - 50 : durationMs,
    16,
  );
}
