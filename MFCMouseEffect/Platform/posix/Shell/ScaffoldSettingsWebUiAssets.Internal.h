#pragma once

#include <string>
#include <string_view>

namespace mousefx::platform::scaffold {

bool IsSafeWebPath(std::string_view path);
std::string NormalizeWebAssetRequestPath(std::string_view requestPath);
std::string ContentTypeForWebPath(std::string_view path);

} // namespace mousefx::platform::scaffold
