#pragma once

#include <gdiplus.h>
#include <string>
#include <vector>

#include "Platform/windows/Pet/Win32MouseCompanionRendererInput.h"

namespace mousefx::windows {

struct Win32MouseCompanionRendererBackendRuntimeDiagnostics {
    std::string backendName;
    bool ready{false};
    bool renderedFrame{false};
    uint64_t renderedFrameCount{0};
    uint64_t lastRenderTickMs{0};
    std::string actionName{"idle"};
    std::string reactiveActionName{"idle"};
    float actionIntensity{0.0f};
    float reactiveActionIntensity{0.0f};
    bool modelReady{false};
    bool actionLibraryReady{false};
    bool appearanceProfileReady{false};
    bool poseFrameAvailable{false};
    bool poseBindingConfigured{false};
    std::string sceneRuntimeAdapterMode{"runtime_only"};
    uint32_t sceneRuntimePoseSampleCount{0};
    uint32_t sceneRuntimeBoundPoseSampleCount{0};
    int facingDirection{1};
    int surfaceWidth{0};
    int surfaceHeight{0};
    std::string modelSourceFormat{"unknown"};
    std::string appearanceSkinVariantId{"default"};
    std::vector<std::string> appearanceAccessoryIds;
    std::string appearanceAccessoryFamily{"none"};
    std::string appearanceComboPreset{"none"};
    std::string appearanceRequestedPresetId;
    std::string appearanceResolvedPresetId;
    std::string appearancePluginId;
    std::string appearancePluginKind;
    std::string appearancePluginSource;
    std::string appearancePluginSelectionReason;
    std::string appearancePluginFailureReason;
    std::string appearancePluginManifestPath;
    std::string appearancePluginRuntimeBackend;
    std::string appearancePluginMetadataPath;
    uint32_t appearancePluginMetadataSchemaVersion{0};
    std::string appearancePluginAppearanceSemanticsMode{"legacy_manifest_compat"};
    std::string appearancePluginSampleTier;
    std::string appearancePluginContractBrief{"legacy_manifest_compat/-/-"};
    std::string defaultLaneCandidate{"builtin"};
    std::string defaultLaneSource{"runtime_builtin_default"};
    std::string defaultLaneRolloutStatus{"stay_on_builtin"};
    std::string defaultLaneStyleIntent{"style_candidate:none"};
    std::string defaultLaneCandidateTier{"builtin_shipped_default"};
};

class IWin32MouseCompanionRendererBackend {
public:
    virtual ~IWin32MouseCompanionRendererBackend() = default;

    virtual bool Start() = 0;
    virtual void Shutdown() = 0;
    virtual bool IsReady() const = 0;
    virtual std::string LastErrorReason() const = 0;
    virtual void Render(
        const Win32MouseCompanionRendererInput& input,
        Gdiplus::Graphics* graphics,
        int width,
        int height) const = 0;
    virtual Win32MouseCompanionRendererBackendRuntimeDiagnostics ReadRuntimeDiagnostics() const = 0;
};

} // namespace mousefx::windows
