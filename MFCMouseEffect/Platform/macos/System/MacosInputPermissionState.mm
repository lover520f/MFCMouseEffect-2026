#include "pch.h"

#include "Platform/macos/System/MacosInputPermissionState.h"

#if defined(__APPLE__)
#import <ApplicationServices/ApplicationServices.h>
#endif

#include <cstdlib>
#include <fstream>
#include <string_view>

namespace mousefx::macos_input_permission {

namespace {

constexpr std::string_view kPermissionSimulationFileEnv =
    "MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE";

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

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

    while (!text.empty() &&
           (text.front() == ' ' || text.front() == '\t' || text.front() == '\r' || text.front() == '\n')) {
        text.remove_prefix(1);
    }
    while (!text.empty() &&
           (text.back() == ' ' || text.back() == '\t' || text.back() == '\r' || text.back() == '\n')) {
        text.remove_suffix(1);
    }
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

} // namespace

std::string ReadPermissionSimulationFilePath() {
    const char* raw = std::getenv(kPermissionSimulationFileEnv.data());
    if (raw == nullptr || raw[0] == '\0') {
        return {};
    }
    return std::string(raw);
}

bool IsRuntimeInputTrusted(const std::string& simulationFilePath) {
    if (!simulationFilePath.empty()) {
        return ReadPermissionSimulationTrusted(simulationFilePath, true);
    }
#if defined(__APPLE__)
    return AXIsProcessTrusted();
#else
    return false;
#endif
}

} // namespace mousefx::macos_input_permission
