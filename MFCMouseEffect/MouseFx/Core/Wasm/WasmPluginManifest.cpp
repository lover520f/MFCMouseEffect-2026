#include "pch.h"

#include "WasmPluginManifest.h"

#include "MouseFx/ThirdParty/json.hpp"
#include "MouseFx/Utils/StringUtils.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cwctype>

namespace mousefx::wasm {

namespace {

using json = nlohmann::json;

bool IsAsciiIdChar(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9') ||
        ch == '.' || ch == '_' || ch == '-';
}

bool IsAsciiIdentifier(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    for (char ch : value) {
        if (!IsAsciiIdChar(ch)) {
            return false;
        }
    }
    return true;
}

std::wstring ToLowerWide(std::wstring text) {
    std::transform(text.begin(), text.end(), text.begin(), [](wchar_t ch) -> wchar_t {
        return static_cast<wchar_t>(::towlower(ch));
    });
    return text;
}

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

    if (!Validate(manifest, &result.error)) {
        return result;
    }

    result.ok = true;
    result.manifest = std::move(manifest);
    return result;
}

bool WasmPluginManifest::Validate(const PluginManifest& manifest, std::string* outError) {
    if (!IsAsciiIdentifier(manifest.id)) {
        if (outError) {
            *outError = "Manifest id must use [A-Za-z0-9._-].";
        }
        return false;
    }
    if (TrimAscii(manifest.name).empty()) {
        if (outError) {
            *outError = "Manifest name must not be empty.";
        }
        return false;
    }
    if (TrimAscii(manifest.version).empty()) {
        if (outError) {
            *outError = "Manifest version must not be empty.";
        }
        return false;
    }
    if (manifest.apiVersion == 0) {
        if (outError) {
            *outError = "Manifest api_version must be >= 1.";
        }
        return false;
    }
    if (manifest.entryWasm.empty()) {
        if (outError) {
            *outError = "Manifest entry must not be empty.";
        }
        return false;
    }
    const std::filesystem::path entryPath(manifest.entryWasm);
    if (entryPath.is_absolute()) {
        if (outError) {
            *outError = "Manifest entry must be relative path.";
        }
        return false;
    }
    if (ToLowerWide(entryPath.extension().wstring()) != L".wasm") {
        if (outError) {
            *outError = "Manifest entry must be a .wasm file.";
        }
        return false;
    }
    if (outError) {
        outError->clear();
    }
    return true;
}

} // namespace mousefx::wasm
