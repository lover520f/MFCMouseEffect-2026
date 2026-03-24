#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendPreference.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendPreferenceRegistry.h"

#include "MouseFx/Utils/StringUtils.h"

#include <cstdlib>

namespace mousefx::windows {
namespace {

constexpr const char* kAutoBackend = "auto";
constexpr const char* kDefaultBackendAlias = "default";
constexpr const char* kBackendEnvVar = "MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND";

std::string ReadEnvCopy(const char* key) {
    if (!key || !*key) {
        return {};
    }
    char* raw = nullptr;
    size_t rawSize = 0;
    const errno_t result = _dupenv_s(&raw, &rawSize, key);
    if (result != 0 || !raw || rawSize == 0) {
        if (raw) {
            free(raw);
        }
        return {};
    }
    std::string value(raw);
    free(raw);
    return value;
}

Win32MouseCompanionRendererBackendPreferenceResolution ResolveConfiguredRendererBackendPreference(
    const Win32MouseCompanionRendererBackendPreferenceRequest& request) {
    if (TrimAscii(request.preferredBackendName).empty()) {
        return {};
    }

    Win32MouseCompanionRendererBackendPreferenceResolution resolution{};
    resolution.matched = true;
    resolution.preference.source =
        TrimAscii(request.preferredBackendSource).empty() ? "configured_request" : request.preferredBackendSource;
    resolution.preference.backendName = NormalizeWin32MouseCompanionRendererBackendName(request.preferredBackendName);
    return resolution;
}

Win32MouseCompanionRendererBackendPreferenceResolution ResolveEnvRendererBackendPreference(
    const Win32MouseCompanionRendererBackendPreferenceRequest&) {
    const std::string raw = ReadEnvCopy(kBackendEnvVar);
    if (TrimAscii(raw).empty()) {
        return {};
    }

    Win32MouseCompanionRendererBackendPreferenceResolution resolution{};
    resolution.matched = true;
    resolution.preference.source = "env:MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND";
    resolution.preference.backendName = NormalizeWin32MouseCompanionRendererBackendName(raw);
    return resolution;
}

Win32MouseCompanionRendererBackendPreferenceResolution ResolveDefaultRendererBackendPreference(
    const Win32MouseCompanionRendererBackendPreferenceRequest&) {
    Win32MouseCompanionRendererBackendPreferenceResolution resolution{};
    resolution.matched = true;
    resolution.preference.source = "default";
    resolution.preference.backendName = "real";
    return resolution;
}

} // namespace

std::string NormalizeWin32MouseCompanionRendererBackendName(const std::string& rawBackendName) {
    const std::string normalized = ToLowerAscii(TrimAscii(rawBackendName));
    if (normalized.empty() || normalized == kDefaultBackendAlias) {
        return kAutoBackend;
    }
    return normalized;
}

bool IsAutoWin32MouseCompanionRendererBackendName(const std::string& backendName) {
    return NormalizeWin32MouseCompanionRendererBackendName(backendName) == kAutoBackend;
}

void RegisterBuiltInWin32MouseCompanionRendererBackendPreferenceSources() {
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    auto& registry = Win32MouseCompanionRendererBackendPreferenceRegistry::Instance();
    registry.Register("configured_request", 200, &ResolveConfiguredRendererBackendPreference);
    registry.Register("env", 100, &ResolveEnvRendererBackendPreference);
    registry.Register("default", 0, &ResolveDefaultRendererBackendPreference);
}

Win32MouseCompanionRendererBackendPreference ResolveWin32MouseCompanionRendererBackendPreference(
    const Win32MouseCompanionRendererBackendPreferenceRequest& request) {
    RegisterBuiltInWin32MouseCompanionRendererBackendPreferenceSources();
    const auto resolution = Win32MouseCompanionRendererBackendPreferenceRegistry::Instance().ResolveHighestPriority(request);
    if (resolution.matched) {
        return resolution.preference;
    }

    Win32MouseCompanionRendererBackendPreference preference{};
    preference.source = "default";
    preference.backendName = "real";
    return preference;
}

Win32MouseCompanionRendererBackendPreference ResolveWin32MouseCompanionRendererBackendPreference() {
    return ResolveWin32MouseCompanionRendererBackendPreference({});
}

Win32MouseCompanionRendererBackendPreference ResolveExplicitWin32MouseCompanionRendererBackendPreference(
    const std::string& preferredBackendName) {
    Win32MouseCompanionRendererBackendPreferenceRequest request{};
    request.preferredBackendSource = "explicit_argument";
    request.preferredBackendName = preferredBackendName;
    return ResolveWin32MouseCompanionRendererBackendPreference(request);
}

} // namespace mousefx::windows
