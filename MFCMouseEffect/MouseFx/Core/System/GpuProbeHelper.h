#pragma once

#include <string>

namespace mousefx {

struct DawnRuntimeProbeResult {
    bool available = false;
    std::string reason = "unknown";
};

// Probe whether the Dawn WebGPU runtime DLL can be loaded.
// Searches exe dir, Runtime/Dawn/ subdirectory, and repo layout.
DawnRuntimeProbeResult ProbeDawnRuntimeOnce();

// Filesystem helpers used by the probe and other startup code.
std::wstring GetExeDirW();
std::wstring ParentDirW(const std::wstring& path);

} // namespace mousefx
