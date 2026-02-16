#pragma once

#include <string>

namespace mousefx {

class QuantumHaloPresenterSelection final {
public:
    static constexpr const char* kAuto = "auto";
    static constexpr const char* kEnvVar = "MFX_QUANTUM_HALO_PRESENTER_BACKEND";

    static void SetConfiguredBackendPreference(const std::string& preference);
    static std::string GetConfiguredBackendPreference();
    static std::string GetEffectiveBackendPreference();
    static std::string NormalizeBackendPreference(const std::string& preference);

private:
    static std::string ReadEnvironmentPreference();
};

} // namespace mousefx
