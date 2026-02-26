#pragma once

#include <cstdint>
#include <string>

namespace mousefx::macos_foreground_process_detail {

uint64_t CurrentSteadyTickMs();
std::string ResolveForegroundProcessBaseName();

} // namespace mousefx::macos_foreground_process_detail
