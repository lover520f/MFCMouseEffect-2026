#pragma once

#include <string>

namespace mousefx::platform {

struct RuntimeProbeResult {
    bool available = false;
    std::string reason = "unknown";
};

std::wstring GetExecutableDirectoryW();
std::wstring GetParentDirectoryW(const std::wstring& path);
RuntimeProbeResult ProbeDawnRuntimeOnce();

} // namespace mousefx::platform
