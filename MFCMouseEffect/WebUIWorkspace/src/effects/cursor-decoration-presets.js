const CURSOR_DECORATION_PRESETS = {
  focus_ring: {
    plugin_id: 'focus_ring',
    color_hex: '#ff5a5a',
    size_px: 22,
    alpha_percent: 82,
  },
  signal_ring: {
    plugin_id: 'signal_ring',
    color_hex: '#ff5548',
    size_px: 28,
    alpha_percent: 90,
  },
  soft_orb: {
    plugin_id: 'soft_orb',
    color_hex: '#ff8a65',
    size_px: 24,
    alpha_percent: 78,
  },
  halo_orb: {
    plugin_id: 'halo_orb',
    color_hex: '#69c6ff',
    size_px: 30,
    alpha_percent: 76,
  },
};

const CURSOR_DECORATION_ALIASES = {
  ring: 'focus_ring',
  orb: 'soft_orb',
};

export function normalizeCursorDecorationPluginId(value) {
  const raw = `${value || ''}`.trim().toLowerCase();
  if (!raw) {
    return 'focus_ring';
  }
  return CURSOR_DECORATION_ALIASES[raw] || raw;
}

export function resolveCursorDecorationPreset(pluginId) {
  const normalized = normalizeCursorDecorationPluginId(pluginId);
  return CURSOR_DECORATION_PRESETS[normalized] || CURSOR_DECORATION_PRESETS.focus_ring;
}

export function buildCursorDecorationFromPreset(pluginId, current = {}) {
  const preset = resolveCursorDecorationPreset(pluginId);
  return {
    ...current,
    enabled: current.enabled !== false,
    plugin_id: preset.plugin_id,
    color_hex: preset.color_hex,
    size_px: preset.size_px,
    alpha_percent: preset.alpha_percent,
  };
}
