export function normalizeEffectsProfile(input) {
  const value = input || {};
  if (!value || typeof value !== 'object') {
    return {};
  }

  const profile = {
    platform: `${value.platform || ''}`.trim(),
    active: value.active && typeof value.active === 'object' ? value.active : {},
  };

  const keys = ['click', 'trail', 'trail_throttle', 'scroll', 'hold', 'hover'];
  for (const key of keys) {
    if (value[key] && typeof value[key] === 'object') {
      profile[key] = value[key];
    }
  }
  return profile;
}
