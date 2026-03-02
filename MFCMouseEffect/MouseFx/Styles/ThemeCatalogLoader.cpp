#include "pch.h"
#include "ThemeCatalogLoader.h"

#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <system_error>

namespace mousefx {
namespace {

using json = nlohmann::json;

std::wstring ToLowerWideAscii(const std::wstring& input) {
    std::wstring out = input;
    std::transform(out.begin(), out.end(), out.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towlower(ch));
    });
    return out;
}

bool EndsWithWide(const std::wstring& value, const std::wstring& suffix) {
    if (suffix.size() > value.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}

bool IsThemeCatalogFile(const std::filesystem::path& path) {
    const std::wstring name = ToLowerWideAscii(path.filename().wstring());
    return name == L"theme.json" || EndsWithWide(name, L".theme.json");
}

bool IsValidThemeValueToken(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    for (char ch : value) {
        const bool isDigit = (ch >= '0' && ch <= '9');
        const bool isLower = (ch >= 'a' && ch <= 'z');
        const bool isSeparator = (ch == '_' || ch == '-');
        if (!isDigit && !isLower && !isSeparator) {
            return false;
        }
    }
    return true;
}

std::vector<std::filesystem::path> DiscoverThemeFiles(const std::wstring& rootPath) {
    std::vector<std::filesystem::path> out;
    if (rootPath.empty()) {
        return out;
    }

    std::error_code ec;
    const std::filesystem::path root(rootPath);
    if (!std::filesystem::exists(root, ec) || ec || !std::filesystem::is_directory(root, ec)) {
        return out;
    }

    std::filesystem::recursive_directory_iterator it(
        root,
        std::filesystem::directory_options::skip_permission_denied,
        ec);
    const std::filesystem::recursive_directory_iterator end;
    while (!ec && it != end) {
        const auto& entry = *it;
        std::error_code localEc;
        if (entry.is_regular_file(localEc) && !localEc && IsThemeCatalogFile(entry.path())) {
            out.push_back(entry.path());
        }
        it.increment(ec);
        if (ec) {
            ec.clear();
        }
    }

    std::sort(out.begin(), out.end(), [](const std::filesystem::path& lhs, const std::filesystem::path& rhs) {
        return ToLowerWideAscii(lhs.wstring()) < ToLowerWideAscii(rhs.wstring());
    });
    return out;
}

bool TryReadFileUtf8(const std::filesystem::path& path, std::string* out) {
    if (!out) {
        return false;
    }
    out->clear();

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();
    if (data.empty()) {
        return false;
    }

    if (data.size() >= 3 &&
        static_cast<unsigned char>(data[0]) == 0xEF &&
        static_cast<unsigned char>(data[1]) == 0xBB &&
        static_cast<unsigned char>(data[2]) == 0xBF) {
        data = data.substr(3);
    }

    if (!IsValidUtf8(data)) {
        data = EnsureUtf8(data);
    }
    if (data.empty()) {
        return false;
    }
    *out = std::move(data);
    return true;
}

bool TryParseColorHex(const std::string& text, Argb* outColor) {
    if (!outColor || text.empty() || text.front() != '#') {
        return false;
    }

    const std::string hex = text.substr(1);
    if (hex.size() != 6 && hex.size() != 8) {
        return false;
    }

    uint32_t value = 0;
    try {
        value = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));
    } catch (...) {
        return false;
    }

    if (hex.size() == 6) {
        value |= 0xFF000000u;
    }
    outColor->value = value;
    return true;
}

bool TryReadInt(const json& source, const char* key, int* out) {
    if (!out || !source.contains(key) || !source[key].is_number_integer()) {
        return false;
    }
    *out = source[key].get<int>();
    return true;
}

bool TryReadUint(const json& source, const char* key, uint32_t* out) {
    if (!out || !source.contains(key) || !source[key].is_number_integer()) {
        return false;
    }
    const int64_t raw = source[key].get<int64_t>();
    if (raw < 0) {
        return false;
    }
    *out = static_cast<uint32_t>(raw);
    return true;
}

bool TryReadFloat(const json& source, const char* key, float* out) {
    if (!out || !source.contains(key) || !source[key].is_number()) {
        return false;
    }
    *out = source[key].get<float>();
    return true;
}

bool TryReadColor(const json& source, const char* key, Argb* out) {
    if (!out || !source.contains(key) || !source[key].is_string()) {
        return false;
    }
    return TryParseColorHex(source[key].get<std::string>(), out);
}

bool ParseRippleStyle(const json& source, RippleStyle* out) {
    if (!out || !source.is_object()) {
        return false;
    }

    RippleStyle style{};
    if (!TryReadUint(source, "duration_ms", &style.durationMs)) {
        return false;
    }
    if (!TryReadInt(source, "window_size", &style.windowSize)) {
        return false;
    }
    if (!TryReadFloat(source, "start_radius", &style.startRadius)) {
        return false;
    }
    if (!TryReadFloat(source, "end_radius", &style.endRadius)) {
        return false;
    }
    if (!TryReadFloat(source, "stroke_width", &style.strokeWidth)) {
        return false;
    }
    if (!TryReadColor(source, "fill", &style.fill)) {
        return false;
    }
    if (!TryReadColor(source, "stroke", &style.stroke)) {
        return false;
    }
    if (!TryReadColor(source, "glow", &style.glow)) {
        return false;
    }

    if (style.windowSize <= 0 || style.endRadius < style.startRadius || style.strokeWidth < 0.0f) {
        return false;
    }
    *out = style;
    return true;
}

bool ParsePalette(const json& source, ThemePalette* out) {
    if (!out || !source.is_object()) {
        return false;
    }
    return ParseRippleStyle(source.value("click", json::object()), &out->click) &&
        ParseRippleStyle(source.value("icon", json::object()), &out->icon) &&
        ParseRippleStyle(source.value("scroll", json::object()), &out->scroll) &&
        ParseRippleStyle(source.value("hold", json::object()), &out->hold) &&
        ParseRippleStyle(source.value("hover", json::object()), &out->hover);
}

bool ParseThemeEntry(const std::filesystem::path& filePath, ThemeCatalogExternalEntry* outEntry) {
    if (!outEntry) {
        return false;
    }

    std::string content;
    if (!TryReadFileUtf8(filePath, &content)) {
        return false;
    }

    json root;
    try {
        root = json::parse(content);
    } catch (...) {
        return false;
    }
    if (!root.is_object()) {
        return false;
    }

    const std::string rawValue = root.value("value", root.value("id", std::string()));
    const std::string normalizedValue = NormalizeThemeName(rawValue);
    if (!IsValidThemeValueToken(normalizedValue) || normalizedValue == "none") {
        return false;
    }

    ThemeCatalogExternalEntry entry;
    entry.value = normalizedValue;
    entry.labelZh = Utf8ToWString(root.value("label_zh", normalizedValue));
    entry.labelEn = Utf8ToWString(root.value("label_en", normalizedValue));

    if (!root.contains("palette")) {
        return false;
    }
    if (!ParsePalette(root["palette"], &entry.palette)) {
        return false;
    }

    *outEntry = std::move(entry);
    return true;
}

} // namespace

ThemeCatalogLoadResult ThemeCatalogLoader::LoadFromRootPath(const std::string& rootPathUtf8) const {
    ThemeCatalogLoadResult result;
    result.configuredRootPath = TrimAscii(rootPathUtf8);
    if (result.configuredRootPath.empty()) {
        return result;
    }

    const std::wstring rootPath = Utf8ToWString(result.configuredRootPath);
    const std::vector<std::filesystem::path> files = DiscoverThemeFiles(rootPath);
    result.scannedFiles = files.size();

    std::set<std::string> seenValues;
    for (const auto& filePath : files) {
        ThemeCatalogExternalEntry entry;
        if (!ParseThemeEntry(filePath, &entry)) {
            ++result.rejectedFiles;
            continue;
        }

        if (seenValues.find(entry.value) != seenValues.end()) {
            ++result.rejectedFiles;
            continue;
        }

        seenValues.insert(entry.value);
        result.entries.push_back(std::move(entry));
    }

    return result;
}

} // namespace mousefx
