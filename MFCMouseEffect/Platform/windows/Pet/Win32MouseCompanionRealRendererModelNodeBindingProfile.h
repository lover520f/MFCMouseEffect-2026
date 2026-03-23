#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererModelNodeBindingEntry final {
    float influence{0.0f};
    float bindWeight{0.0f};
    float localOffsetX{0.0f};
    float localOffsetY{0.0f};
    float worldOffsetX{0.0f};
    float worldOffsetY{0.0f};
    bool bound{false};
};

struct Win32MouseCompanionRealRendererModelNodeBindingProfile final {
    std::string bindingState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t boundEntryCount{0};
    Win32MouseCompanionRealRendererModelNodeBindingEntry bodyEntry{};
    Win32MouseCompanionRealRendererModelNodeBindingEntry headEntry{};
    Win32MouseCompanionRealRendererModelNodeBindingEntry appendageEntry{};
    Win32MouseCompanionRealRendererModelNodeBindingEntry overlayEntry{};
    Win32MouseCompanionRealRendererModelNodeBindingEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string weightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
};

Win32MouseCompanionRealRendererModelNodeBindingProfile
BuildWin32MouseCompanionRealRendererModelNodeBindingProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
