#pragma once

#include <string_view>

namespace mousefx::platform {

bool IsPosixShellExitCommandLine(std::string_view line);

} // namespace mousefx::platform
