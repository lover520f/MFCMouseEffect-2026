// SettingsSchemaBuilder.cpp -- top-level schema composition.

#include "pch.h"
#include "SettingsSchemaBuilder.h"

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Server/SettingsSchemaBuilder.CapabilitiesSections.h"
#include "MouseFx/Server/SettingsSchemaBuilder.OptionsSections.h"

using json = nlohmann::json;

namespace mousefx {

std::string BuildSettingsSchemaJson(const EffectConfig& config) {
    json out = json::object();
    AppendSettingsSchemaOptionsSections(config, &out);
    AppendSettingsSchemaCapabilitiesSections(config, &out);
    return out.dump();
}

} // namespace mousefx
