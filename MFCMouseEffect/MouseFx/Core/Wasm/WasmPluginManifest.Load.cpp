#include "pch.h"

#include "WasmPluginManifest.h"

#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Utils/StringUtils.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace mousefx::wasm {
namespace {

using json = nlohmann::json;

std::string ReadTextFileUtf8(const std::filesystem::path& filePath, std::string* outError) {
    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open()) {
        if (outError) {
            *outError = "Cannot open plugin manifest.";
        }
        return {};
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    if (input.bad()) {
        if (outError) {
            *outError = "Failed reading plugin manifest.";
        }
        return {};
    }
    return buffer.str();
}

bool TryReadStringField(const json& node, const char* key, std::string* outValue, std::string* outError) {
    if (!outValue) {
        return false;
    }
    if (!node.contains(key) || !node[key].is_string()) {
        if (outError) {
            *outError = std::string("Manifest field '") + key + "' must be string.";
        }
        return false;
    }
    *outValue = node[key].get<std::string>();
    return true;
}

bool TryReadUintField(const json& node, const char* key, uint32_t* outValue, std::string* outError) {
    if (!outValue) {
        return false;
    }
    if (!node.contains(key) || !node[key].is_number_unsigned()) {
        if (outError) {
            *outError = std::string("Manifest field '") + key + "' must be unsigned number.";
        }
        return false;
    }
    *outValue = node[key].get<uint32_t>();
    return true;
}

bool TryReadImageAssetsField(
    const json& node,
    std::vector<std::wstring>* outAssets,
    std::string* outError) {
    if (!outAssets) {
        return false;
    }
    outAssets->clear();
    if (!node.contains("image_assets")) {
        return true;
    }
    if (!node["image_assets"].is_array()) {
        if (outError) {
            *outError = "Manifest field 'image_assets' must be string array.";
        }
        return false;
    }
    for (const auto& item : node["image_assets"]) {
        if (!item.is_string()) {
            if (outError) {
                *outError = "Manifest field 'image_assets' must be string array.";
            }
            return false;
        }
        outAssets->push_back(Utf8ToWString(item.get<std::string>()));
    }
    return true;
}

std::string NormalizeInputKindToken(std::string token) {
    token = ToLowerAscii(TrimAscii(token));
    for (char& ch : token) {
        if (ch == '-') {
            ch = '_';
        }
    }
    return token;
}

bool TryReadInputKindsField(
    const json& node,
    uint32_t* outMask,
    std::string* outError) {
    if (!outMask) {
        return false;
    }
    if (!node.contains("input_kinds")) {
        return true;
    }
    if (!node["input_kinds"].is_array()) {
        if (outError) {
            *outError = "Manifest field 'input_kinds' must be string array.";
        }
        return false;
    }

    uint32_t mask = 0u;
    bool hasKind = false;
    for (const auto& item : node["input_kinds"]) {
        if (!item.is_string()) {
            if (outError) {
                *outError = "Manifest field 'input_kinds' must be string array.";
            }
            return false;
        }
        const std::string token = NormalizeInputKindToken(item.get<std::string>());
        if (token.empty()) {
            if (outError) {
                *outError = "Manifest field 'input_kinds' must not contain empty item.";
            }
            return false;
        }
        if (token == "all") {
            mask = kManifestInputKindAllBits;
            hasKind = true;
            break;
        }
        if (token == "click") {
            mask |= kManifestInputKindClickBit;
        } else if (token == "move") {
            mask |= kManifestInputKindMoveBit;
        } else if (token == "scroll") {
            mask |= kManifestInputKindScrollBit;
        } else if (token == "hold_start" || token == "holdstart") {
            mask |= kManifestInputKindHoldStartBit;
        } else if (token == "hold_update" || token == "holdupdate") {
            mask |= kManifestInputKindHoldUpdateBit;
        } else if (token == "hold_end" || token == "holdend") {
            mask |= kManifestInputKindHoldEndBit;
        } else if (token == "hover_start" || token == "hoverstart") {
            mask |= kManifestInputKindHoverStartBit;
        } else if (token == "hover_end" || token == "hoverend") {
            mask |= kManifestInputKindHoverEndBit;
        } else if (token == "indicator_click" || token == "indicatorclick") {
            mask |= kManifestInputKindIndicatorClickBit;
        } else if (token == "indicator_scroll" || token == "indicatorscroll") {
            mask |= kManifestInputKindIndicatorScrollBit;
        } else if (token == "indicator_key" || token == "indicatorkey") {
            mask |= kManifestInputKindIndicatorKeyBit;
        } else {
            if (outError) {
                *outError = std::string("Manifest field 'input_kinds' contains unsupported value: ") + token;
            }
            return false;
        }
        hasKind = true;
    }

    if (!hasKind || mask == 0u) {
        if (outError) {
            *outError = "Manifest field 'input_kinds' must include at least one input kind.";
        }
        return false;
    }
    *outMask = mask;
    return true;
}

bool TryReadSurfaceKindsField(
    const json& node,
    uint32_t* outMask,
    bool* outHasExplicitMask,
    std::string* outError) {
    if (!outMask || !outHasExplicitMask) {
        return false;
    }
    if (!node.contains("surfaces")) {
        *outHasExplicitMask = false;
        return true;
    }
    if (!node["surfaces"].is_array()) {
        if (outError) {
            *outError = "Manifest field 'surfaces' must be string array.";
        }
        return false;
    }

    uint32_t mask = 0u;
    bool hasSurface = false;
    for (const auto& item : node["surfaces"]) {
        if (!item.is_string()) {
            if (outError) {
                *outError = "Manifest field 'surfaces' must be string array.";
            }
            return false;
        }
        const std::string token = NormalizeInputKindToken(item.get<std::string>());
        if (token.empty()) {
            if (outError) {
                *outError = "Manifest field 'surfaces' must not contain empty item.";
            }
            return false;
        }
        if (token == "all") {
            mask = kManifestSurfaceAllBits;
            hasSurface = true;
            break;
        }
        if (token == "effects" || token == "effect") {
            mask |= kManifestSurfaceEffectsBit;
        } else if (
            token == "indicator" ||
            token == "input_indicator" ||
            token == "inputindicator") {
            mask |= kManifestSurfaceIndicatorBit;
        } else {
            if (outError) {
                *outError = std::string("Manifest field 'surfaces' contains unsupported value: ") + token;
            }
            return false;
        }
        hasSurface = true;
    }

    if (!hasSurface || mask == 0u) {
        if (outError) {
            *outError = "Manifest field 'surfaces' must include at least one surface.";
        }
        return false;
    }
    *outMask = mask;
    *outHasExplicitMask = true;
    return true;
}

bool TryReadEnableFrameTickField(const json& node, bool* outEnable, std::string* outError) {
    if (!outEnable) {
        return false;
    }
    if (!node.contains("enable_frame_tick")) {
        return true;
    }
    if (!node["enable_frame_tick"].is_boolean()) {
        if (outError) {
            *outError = "Manifest field 'enable_frame_tick' must be boolean.";
        }
        return false;
    }
    *outEnable = node["enable_frame_tick"].get<bool>();
    return true;
}

} // namespace

PluginManifestLoadResult WasmPluginManifest::LoadFromFile(const std::wstring& manifestPath) {
    PluginManifestLoadResult result{};
    const std::filesystem::path path(manifestPath);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec) {
        result.error = "Manifest file does not exist.";
        return result;
    }

    std::string readError;
    const std::string content = ReadTextFileUtf8(path, &readError);
    if (content.empty()) {
        result.error = readError.empty() ? "Manifest file is empty." : readError;
        return result;
    }

    json root;
    try {
        root = json::parse(content);
    } catch (const std::exception& ex) {
        result.error = std::string("Manifest JSON parse error: ") + ex.what();
        return result;
    }
    if (!root.is_object()) {
        result.error = "Manifest root must be a JSON object.";
        return result;
    }

    PluginManifest manifest{};
    if (!TryReadStringField(root, "id", &manifest.id, &result.error)) {
        return result;
    }
    if (!TryReadStringField(root, "name", &manifest.name, &result.error)) {
        return result;
    }
    if (!TryReadStringField(root, "version", &manifest.version, &result.error)) {
        return result;
    }
    if (!TryReadUintField(root, "api_version", &manifest.apiVersion, &result.error)) {
        return result;
    }

    std::string entryUtf8;
    if (!TryReadStringField(root, "entry", &entryUtf8, &result.error)) {
        return result;
    }
    manifest.entryWasm = Utf8ToWString(entryUtf8);
    if (!TryReadImageAssetsField(root, &manifest.imageAssets, &result.error)) {
        return result;
    }
    if (!TryReadInputKindsField(root, &manifest.inputKindsMask, &result.error)) {
        return result;
    }
    if (!TryReadSurfaceKindsField(
            root,
            &manifest.surfaceKindsMask,
            &manifest.hasExplicitSurfaceKinds,
            &result.error)) {
        return result;
    }
    if (!TryReadEnableFrameTickField(root, &manifest.enableFrameTick, &result.error)) {
        return result;
    }

    if (!Validate(manifest, &result.error)) {
        return result;
    }

    result.ok = true;
    result.manifest = std::move(manifest);
    return result;
}

} // namespace mousefx::wasm
