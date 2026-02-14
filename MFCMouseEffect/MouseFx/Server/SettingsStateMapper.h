#pragma once

#include <string>

namespace mousefx {

class AppController;
struct EffectConfig;

// Build the current settings state as JSON for the WebUI.
std::string BuildSettingsStateJson(const EffectConfig& config);

// Apply a state JSON payload by forwarding an apply_settings command.
// Returns a JSON response string { "ok": true/false }.
std::string ApplySettingsStateJson(AppController* controller, const std::string& body);

} // namespace mousefx
