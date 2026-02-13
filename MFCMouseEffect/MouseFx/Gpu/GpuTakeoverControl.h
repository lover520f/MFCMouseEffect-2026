#pragma once

#include <filesystem>
#include <string>

namespace mousefx::gpu {

struct TakeoverControlDecision {
    bool takeoverEnabled = false;
    bool visibleTrialEnabled = false;
    bool rearmProcessed = false;
    bool onFilePresent = false;
    bool offFilePresent = false;
    bool autoOffFilePresent = false;
    bool visibleTrialFilePresent = false;
    bool onceFilePresent = false;
    bool onceFileConsumed = false;
    bool visibleTrialOnceFilePresent = false;
    bool visibleTrialOnceFileConsumed = false;
    bool visibleTrialDowngradedByMultiMonitor = false;
    std::string source = "default_off";
    std::string detail = "no_control_file_or_env";
};

std::filesystem::path ResolveGpuDiagDirFromCurrentModule();
void WriteGpuAutoDisableMarker(const char* reason);
TakeoverControlDecision ResolveTakeoverControlDecision();
bool ConsumeOneShotControlFileForSource(const char* source);

} // namespace mousefx::gpu
