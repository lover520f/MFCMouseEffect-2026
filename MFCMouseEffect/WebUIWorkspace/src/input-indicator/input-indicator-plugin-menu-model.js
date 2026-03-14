export function supportsIndicatorKinds(inputKinds, surfaces, hasExplicitSurfaces) {
  const normalizedSurfaces = Array.isArray(surfaces)
    ? surfaces.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
    : [];
  if (hasExplicitSurfaces) {
    return normalizedSurfaces.includes('indicator');
  }
  const kinds = Array.isArray(inputKinds)
    ? inputKinds.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
    : [];
  if (kinds.length === 0) {
    return true;
  }
  return kinds.some((entry) => entry.startsWith('indicator_'));
}

export function normalizeCatalogItems(input) {
  const source = Array.isArray(input) ? input : [];
  const out = [];
  for (const item of source) {
    const value = item || {};
    const manifestPath = `${value.manifest_path || ''}`.trim();
    if (!manifestPath || !supportsIndicatorKinds(value.input_kinds, value.surfaces, value.has_explicit_surfaces)) {
      continue;
    }
    out.push({
      id: `${value.id || ''}`.trim(),
      name: `${value.name || ''}`.trim(),
      version: `${value.version || ''}`.trim(),
      surfaces: Array.isArray(value.surfaces)
        ? value.surfaces.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
        : [],
      has_explicit_surfaces: !!value.has_explicit_surfaces,
      input_kinds: Array.isArray(value.input_kinds)
        ? value.input_kinds.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
        : [],
      manifest_path: manifestPath,
    });
  }
  return out;
}

export function normalizeCatalogErrors(input) {
  const source = Array.isArray(input) ? input : [];
  const out = [];
  for (const item of source) {
    const value = `${item || ''}`.trim();
    if (!value) {
      continue;
    }
    out.push(value);
  }
  return out;
}

export function normalizeManifestPathForCompare(path) {
  const value = `${path || ''}`.trim();
  if (!value) {
    return '';
  }
  return value.replace(/\\/g, '/').replace(/\/+/g, '/').toLowerCase();
}

export function manifestPathForIndicatorLoad(selectedManifestPath, currentManifestPath) {
  const selected = `${selectedManifestPath || ''}`.trim();
  if (selected) {
    return selected;
  }
  return `${currentManifestPath || ''}`.trim();
}

export function findCatalogItemByManifestPath(items, manifestPath) {
  const expected = normalizeManifestPathForCompare(manifestPath);
  if (!expected) {
    return null;
  }
  for (const item of items || []) {
    if (normalizeManifestPathForCompare(item.manifest_path) === expected) {
      return item;
    }
  }
  return null;
}

export function normalizePluginIdForCompare(pluginId) {
  return `${pluginId || ''}`.trim().toLowerCase();
}

export function findCatalogItemByPluginId(items, pluginId) {
  const expected = normalizePluginIdForCompare(pluginId);
  if (!expected) {
    return null;
  }
  for (const item of items || []) {
    if (normalizePluginIdForCompare(item.id) === expected) {
      return item;
    }
  }
  return null;
}

export function pluginLabel(plugin, textFn) {
  const title = plugin?.name || plugin?.id || textFn?.('wasm_text_unknown_plugin', 'Unknown plugin');
  if (!plugin?.version) {
    return title;
  }
  return `${title} (${plugin.version})`;
}

export function normalizeRouteStatus(input) {
  const value = input && typeof input === 'object' ? input : {};
  return {
    route_attempted: !!value.route_attempted,
    event_kind: `${value.event_kind || ''}`.trim(),
    reason: `${value.reason || ''}`.trim(),
    rendered_by_wasm: !!value.rendered_by_wasm,
    native_fallback_applied: !!value.native_fallback_applied,
    event_supported: !!value.event_supported,
    wasm_fallback_enabled: !!value.wasm_fallback_enabled,
  };
}

export function routeEventText(kind, textFn) {
  switch (kind) {
    case 'click':
      return textFn?.('text_input_indicator_route_event_click', 'Click');
    case 'scroll':
      return textFn?.('text_input_indicator_route_event_scroll', 'Scroll');
    case 'key':
      return textFn?.('text_input_indicator_route_event_key', 'Key');
    default:
      return textFn?.('text_input_indicator_route_event_unknown', 'Unknown');
  }
}

export function routeReasonText(reason, textFn) {
  switch (reason) {
    case 'wasm_rendered':
      return textFn?.('text_input_indicator_route_reason_wasm_rendered', 'WASM rendered output');
    case 'fallback_disabled':
      return textFn?.('text_input_indicator_route_reason_fallback_disabled', 'Fallback disabled');
    case 'event_not_supported':
      return textFn?.('text_input_indicator_route_reason_event_not_supported', 'Plugin does not support this event');
    case 'plugin_unloaded':
      return textFn?.('text_input_indicator_route_reason_plugin_unloaded', 'Runtime or plugin not ready');
    case 'anchor_unavailable':
      return textFn?.('text_input_indicator_route_reason_anchor_unavailable', 'Indicator anchor unavailable');
    case 'invoke_failed_no_output':
      return textFn?.('text_input_indicator_route_reason_invoke_failed_no_output', 'Plugin invoked but no output');
    case 'invoke_no_output':
      return textFn?.('text_input_indicator_route_reason_invoke_no_output', 'Plugin invoked with no visible output');
    default:
      return textFn?.('text_input_indicator_route_reason_unknown', 'Unknown');
  }
}
