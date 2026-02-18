export function seedFromTickMs(tickMs: u64): u32 {
  return <u32>(tickMs & 0xFFFFFFFF);
}

export function colorFromSeed(seed: u32): u32 {
  const r: u32 = (seed >> 0) & 0xFF;
  const g: u32 = (seed >> 8) & 0xFF;
  const b: u32 = (seed >> 16) & 0xFF;
  return 0xFF000000 | (r << 16) | (g << 8) | b;
}

export function signedFromSeed(seed: u32, shift: u32, span: i32): i32 {
  const width = span <= 0 ? 1 : span;
  const raw = <i32>((seed >> shift) % <u32>(width * 2 + 1));
  return raw - width;
}

export function rangedFromSeed(seed: u32, shift: u32, min: i32, max: i32): i32 {
  const clampedMax = max < min ? min : max;
  const width = clampedMax - min + 1;
  const raw = <i32>((seed >> shift) % <u32>(width));
  return min + raw;
}
