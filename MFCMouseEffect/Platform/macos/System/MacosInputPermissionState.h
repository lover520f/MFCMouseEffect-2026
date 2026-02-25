#pragma once

#include <string>

namespace mousefx::macos_input_permission {

std::string ReadPermissionSimulationFilePath();
bool IsRuntimeInputTrusted(const std::string& simulationFilePath);

} // namespace mousefx::macos_input_permission
