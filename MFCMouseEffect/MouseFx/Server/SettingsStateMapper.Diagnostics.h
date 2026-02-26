#pragma once

#include <string>

#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx {

class AppController;
struct EffectConfig;

nlohmann::json ReadGpuRouteStatusSnapshot();
nlohmann::json BuildGpuRouteNotice(
    const nlohmann::json& routeStatus,
    const std::string& lang,
    const std::string& activeHoldType);
nlohmann::json BuildWasmState(const EffectConfig& cfg, const AppController* controller);
nlohmann::json BuildEffectsRuntimeState();
nlohmann::json BuildEffectsProfileState(const EffectConfig& cfg);
nlohmann::json BuildInputCaptureState(const AppController* controller, const std::string& lang);

} // namespace mousefx
