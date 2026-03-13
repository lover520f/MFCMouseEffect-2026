#pragma once

#include <string>

namespace mousefx {

struct EffectConfig;

// Build the settings schema JSON for the WebUI.
// The schema contains option lists (effects, themes, languages, monitors, etc.)
// with labels localized based on the config's UI language.
std::string BuildSettingsSchemaJson(const EffectConfig& config);

} // namespace mousefx
