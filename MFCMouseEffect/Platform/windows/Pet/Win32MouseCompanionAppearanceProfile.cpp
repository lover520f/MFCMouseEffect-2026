#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionAppearanceProfile.h"

#include "MouseFx/Core/Json/JsonFacade.h"

#include <fstream>

namespace mousefx::windows {
namespace {

using json = nlohmann::json;

bool ApplyAppearanceNode(
    const json& node,
    const std::string& resolvedPresetId,
    Win32MouseCompanionAppearanceProfile* outProfile) {
    if (!outProfile || !node.is_object()) {
        return false;
    }
    outProfile->skinVariantId = node.value("skinVariantId", "default");
    outProfile->enabledAccessoryIds.clear();
    if (node.contains("enabledAccessoryIds") && node["enabledAccessoryIds"].is_array()) {
        for (const auto& item : node["enabledAccessoryIds"]) {
            if (item.is_string()) {
                outProfile->enabledAccessoryIds.push_back(item.get<std::string>());
            }
        }
    }
    outProfile->resolvedPresetId = resolvedPresetId;
    outProfile->loaded = true;
    return true;
}

} // namespace

bool LoadWin32MouseCompanionAppearanceProfileFromPath(
    const std::string& path,
    Win32MouseCompanionAppearanceProfile* outProfile) {
    if (!outProfile) {
        return false;
    }
    *outProfile = {};

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    json root;
    try {
        root = json::parse(content);
    } catch (...) {
        return false;
    }

    if (root.contains("activePreset") && root["activePreset"].is_string()) {
        outProfile->requestedPresetId = root["activePreset"].get<std::string>();
    }

    if (root.contains("activePreset") &&
        root["activePreset"].is_string() &&
        root.contains("presets") &&
        root["presets"].is_array()) {
        const std::string activePreset = root["activePreset"].get<std::string>();
        for (const auto& preset : root["presets"]) {
            if (!preset.is_object() || !preset.contains("id") || !preset["id"].is_string()) {
                continue;
            }
            if (preset["id"].get<std::string>() == activePreset) {
                return ApplyAppearanceNode(preset, activePreset, outProfile);
            }
        }
    }

    if (root.contains("default")) {
        return ApplyAppearanceNode(root["default"], "default", outProfile);
    }

    return false;
}

} // namespace mousefx::windows
