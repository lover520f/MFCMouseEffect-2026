#include "pch.h"

#include "Platform/macos/System/MacosInputPermissionState.Internal.h"

#include <fstream>

namespace mousefx::macos_input_permission::permission_detail {
namespace {

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

void TrimAsciiWhitespace(std::string_view* text) {
    if (!text) {
        return;
    }
    while (!text->empty() && (text->front() == ' ' || text->front() == '\t' || text->front() == '\r' ||
                               text->front() == '\n')) {
        text->remove_prefix(1);
    }
    while (!text->empty() && (text->back() == ' ' || text->back() == '\t' || text->back() == '\r' ||
                               text->back() == '\n')) {
        text->remove_suffix(1);
    }
}

} // namespace

bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (ToLowerAscii(lhs[i]) != ToLowerAscii(rhs[i])) {
            return false;
        }
    }
    return true;
}

bool ParseBoolText(std::string_view text, bool* outValue) {
    if (!outValue) {
        return false;
    }

    TrimAsciiWhitespace(&text);
    if (text.empty()) {
        return false;
    }

    if (text == "1" || EqualsIgnoreCaseAscii(text, "true") || EqualsIgnoreCaseAscii(text, "yes") ||
        EqualsIgnoreCaseAscii(text, "on")) {
        *outValue = true;
        return true;
    }
    if (text == "0" || EqualsIgnoreCaseAscii(text, "false") || EqualsIgnoreCaseAscii(text, "no") ||
        EqualsIgnoreCaseAscii(text, "off")) {
        *outValue = false;
        return true;
    }
    return false;
}

bool ReadPermissionSimulationTrusted(const std::string& filePath, bool defaultTrusted) {
    if (filePath.empty()) {
        return defaultTrusted;
    }

    std::ifstream in(filePath);
    if (!in.is_open()) {
        return defaultTrusted;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        const size_t split = line.find('=');
        if (split == std::string::npos) {
            bool parsedValue = defaultTrusted;
            if (ParseBoolText(line, &parsedValue)) {
                return parsedValue;
            }
            continue;
        }

        const std::string_view key(line.data(), split);
        const std::string_view value(line.data() + split + 1, line.size() - split - 1);
        if (EqualsIgnoreCaseAscii(key, "trusted")) {
            bool parsedValue = defaultTrusted;
            if (ParseBoolText(value, &parsedValue)) {
                return parsedValue;
            }
        }
    }

    return defaultTrusted;
}

} // namespace mousefx::macos_input_permission::permission_detail
