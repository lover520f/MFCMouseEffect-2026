#pragma once

#include <string>
#include <string_view>

namespace mousefx::macos_input_permission::permission_detail {

bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs);
bool ParseBoolText(std::string_view text, bool* outValue);
bool ReadPermissionSimulationTrusted(const std::string& filePath, bool defaultTrusted);

} // namespace mousefx::macos_input_permission::permission_detail
