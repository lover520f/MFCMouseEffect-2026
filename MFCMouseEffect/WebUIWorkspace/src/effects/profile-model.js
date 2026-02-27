export function normalizeEffectsProfile(input) {
  const value = input || {};
  if (!value || typeof value !== 'object') {
    return {};
  }

  const normalizeSection = (section) => (section && typeof section === 'object' ? section : undefined);

  const profile = {
    platform: `${value.platform || ''}`.trim(),
    active: value.active && typeof value.active === 'object' ? value.active : {},
  };

  const keys = ['config_basis', 'click', 'trail', 'trail_throttle', 'scroll', 'hold', 'hover'];
  for (const key of keys) {
    const section = normalizeSection(value[key]);
    if (section) {
      profile[key] = section;
    }
  }
  return profile;
}
