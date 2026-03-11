import {
  PATH_FILL_RULE_EVEN_ODD,
  PATH_FILL_RULE_NON_ZERO,
  writePathStrokeNodeClose,
  writePathStrokeNodeLineTo,
  writePathStrokeNodeMoveTo,
  writeSpawnPathFillHeaderWithSemantics,
} from "./abi";

export const PATH_BADGE_NODE_COUNT: u32 = 10;
export const PATH_BADGE_SPARK_NODE_COUNT: u32 = 5;

export function emitPathBadge(
  outputPtr: usize,
  x: f32,
  y: f32,
  radius: f32,
  innerRadius: f32,
  skew: f32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnPathFillHeaderWithSemantics(
    outputPtr,
    PATH_BADGE_NODE_COUNT,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    delayMs,
    lifeMs,
    PATH_FILL_RULE_EVEN_ODD,
    blendMode,
    sortKey,
    groupId,
  );

  writePathStrokeNodeMoveTo(outputPtr, 0, x, y - radius);
  writePathStrokeNodeLineTo(outputPtr, 1, x + radius + skew, y);
  writePathStrokeNodeLineTo(outputPtr, 2, x, y + radius);
  writePathStrokeNodeLineTo(outputPtr, 3, x - radius + skew, y);
  writePathStrokeNodeClose(outputPtr, 4);

  writePathStrokeNodeMoveTo(outputPtr, 5, x, y - innerRadius);
  writePathStrokeNodeLineTo(outputPtr, 6, x + innerRadius, y);
  writePathStrokeNodeLineTo(outputPtr, 7, x, y + innerRadius);
  writePathStrokeNodeLineTo(outputPtr, 8, x - innerRadius, y);
  writePathStrokeNodeClose(outputPtr, 9);
}

export function emitPathBadgeSpark(
  outputPtr: usize,
  x: f32,
  y: f32,
  spread: f32,
  lift: f32,
  tilt: f32,
  alpha: f32,
  glowWidthPx: f32,
  fillArgb: u32,
  glowArgb: u32,
  delayMs: u32,
  lifeMs: u32,
  blendMode: u32,
  sortKey: i32,
  groupId: u32,
): void {
  writeSpawnPathFillHeaderWithSemantics(
    outputPtr,
    PATH_BADGE_SPARK_NODE_COUNT,
    alpha,
    glowWidthPx,
    fillArgb,
    glowArgb,
    delayMs,
    lifeMs,
    PATH_FILL_RULE_NON_ZERO,
    blendMode,
    sortKey,
    groupId,
  );

  writePathStrokeNodeMoveTo(outputPtr, 0, x + tilt, y - lift);
  writePathStrokeNodeLineTo(outputPtr, 1, x + spread, y + 2.0);
  writePathStrokeNodeLineTo(outputPtr, 2, x + tilt * 0.5, y + 12.0);
  writePathStrokeNodeLineTo(outputPtr, 3, x - spread * 0.36, y - 4.0);
  writePathStrokeNodeClose(outputPtr, 4);
}
