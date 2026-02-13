#pragma once

#include <filesystem>
#include <string>

namespace mousefx::gpu {

struct TakeoverControlDecision {
    bool takeoverEnabled = false;
    bool visibleTrialEnabled = false;
    bool rearmProcessed = false;
    std::string source = "default_off";
    std::string detail = "no_control_file_or_env";
};

std::filesystem::path ResolveGpuDiagDirFromCurrentModule();
void WriteGpuAutoDisableMarker(const char* reason);
TakeoverControlDecision ResolveTakeoverControlDecision();

} // namespace mousefx::gpu

