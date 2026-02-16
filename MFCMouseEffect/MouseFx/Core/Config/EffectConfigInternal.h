#pragma once

#include "EffectConfig.h"

#include <string>

namespace mousefx::config_internal {

std::string ReadFileAsUtf8(const std::wstring& path);
std::string WStringToUtf8(const std::wstring& ws);
std::string ArgbToHex(Argb color);

std::string NormalizeHoldFollowMode(std::string mode);
std::string NormalizeHoldPresenterBackend(std::string backend);
TrailHistoryProfile SanitizeTrailHistoryProfile(TrailHistoryProfile profile);
TrailRendererParamsConfig SanitizeTrailParams(TrailRendererParamsConfig params);
InputIndicatorConfig SanitizeInputIndicatorConfig(InputIndicatorConfig config);
InputAutomationConfig SanitizeInputAutomationConfig(InputAutomationConfig config);

} // namespace mousefx::config_internal
