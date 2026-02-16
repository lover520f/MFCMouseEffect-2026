#pragma once

#include "EffectConfig.h"
#include "MouseFx/ThirdParty/json.hpp"

namespace mousefx::config_json::effects_color_helpers {

void ParseRippleButtonColors(const nlohmann::json& section, RippleConfig::ButtonColors& colors);
void ParseFillStrokeColors(const nlohmann::json& section, Argb& fill, Argb& stroke);

nlohmann::json BuildRippleButtonColors(const RippleConfig::ButtonColors& colors);
nlohmann::json BuildFillStrokeColors(Argb fill, Argb stroke);

} // namespace mousefx::config_json::effects_color_helpers
