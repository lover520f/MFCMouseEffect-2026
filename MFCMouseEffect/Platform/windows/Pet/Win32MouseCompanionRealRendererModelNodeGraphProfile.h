#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererModelNodeGraphNode final {
    float influence{0.0f};
    float localOffsetX{0.0f};
    float localOffsetY{0.0f};
    float worldOffsetX{0.0f};
    float worldOffsetY{0.0f};
};

struct Win32MouseCompanionRealRendererModelNodeGraphProfile final {
    std::string graphState{"preview_only"};
    uint32_t nodeCount{0};
    uint32_t boundNodeCount{0};
    Win32MouseCompanionRealRendererModelNodeGraphNode bodyNode{};
    Win32MouseCompanionRealRendererModelNodeGraphNode headNode{};
    Win32MouseCompanionRealRendererModelNodeGraphNode appendageNode{};
    Win32MouseCompanionRealRendererModelNodeGraphNode overlayNode{};
    Win32MouseCompanionRealRendererModelNodeGraphNode groundingNode{};
    std::string brief{"preview_only/0/0"};
};

Win32MouseCompanionRealRendererModelNodeGraphProfile
BuildWin32MouseCompanionRealRendererModelNodeGraphProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
