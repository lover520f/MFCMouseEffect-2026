#include "pch.h"
#include "QuantumHaloPresenterSelection.h"

#include "MouseFx/Utils/StringUtils.h"

#include <mutex>

namespace mousefx {
namespace {

std::mutex gSelectionMutex{};
std::string gConfiguredPreference = QuantumHaloPresenterSelection::kAuto;

} // namespace

void QuantumHaloPresenterSelection::SetConfiguredBackendPreference(const std::string& preference) {
    const std::string normalized = NormalizeBackendPreference(preference);
    std::lock_guard<std::mutex> lock(gSelectionMutex);
    gConfiguredPreference = normalized;
}

std::string QuantumHaloPresenterSelection::GetConfiguredBackendPreference() {
    std::lock_guard<std::mutex> lock(gSelectionMutex);
    return gConfiguredPreference;
}

std::string QuantumHaloPresenterSelection::GetEffectiveBackendPreference() {
    const std::string envValue = ReadEnvironmentPreference();
    if (!envValue.empty() && envValue != kAuto) {
        return envValue;
    }
    return GetConfiguredBackendPreference();
}

std::string QuantumHaloPresenterSelection::NormalizeBackendPreference(const std::string& preference) {
    const std::string normalized = ToLowerAscii(TrimAscii(preference));
    if (normalized.empty()) {
        return kAuto;
    }
    return normalized;
}

std::string QuantumHaloPresenterSelection::ReadEnvironmentPreference() {
    char value[256] = {};
    const DWORD len = GetEnvironmentVariableA(kEnvVar, value, static_cast<DWORD>(sizeof(value)));
    if (len == 0 || len >= sizeof(value)) {
        return {};
    }
    return NormalizeBackendPreference(std::string(value, value + len));
}

} // namespace mousefx
