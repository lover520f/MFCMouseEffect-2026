#pragma once

#include "MouseFx/Core/Json/JsonFacade.h"

#include <string>

namespace mousefx {

class AppController;

namespace command_handler_apply_settings {

using json = nlohmann::json;

bool TryParsePayloadObject(const std::string& jsonCmd, json* outPayload);

void ApplyActiveSettings(const json& payload, AppController* controller);
void ApplyTextSettings(const json& payload, AppController* controller);
void ApplyInputIndicatorSettings(const json& payload, AppController* controller);
void ApplyAutomationSettings(const json& payload, AppController* controller);
void ApplyWasmSettings(const json& payload, AppController* controller);
void ApplyTrailTuningSettings(const json& payload, AppController* controller);
void ApplyEffectSizeScaleSettings(const json& payload, AppController* controller);
void ApplyEffectConflictPolicySettings(const json& payload, AppController* controller);

} // namespace command_handler_apply_settings
} // namespace mousefx
