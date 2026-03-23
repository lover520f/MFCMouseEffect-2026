#pragma once

#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"

namespace mousefx {

struct ConfiguredMouseCompanionRendererBackendPreferenceDiagnostics {
    bool effective{false};
    std::string status;
};

struct MouseCompanionRealRendererPreviewDiagnostics {
    bool rolloutEnabled{false};
    bool previewSelected{false};
    bool previewActive{false};
    bool renderedFrame{false};
    uint64_t renderedFrameCount{0};
    uint64_t lastRenderTickMs{0};
    std::string availabilityReason;
    bool modelReady{false};
    bool actionLibraryReady{false};
    bool appearanceProfileReady{false};
    bool poseFrameAvailable{false};
    bool poseBindingConfigured{false};
    std::string sceneRuntimeAdapterMode{"runtime_only"};
    uint32_t sceneRuntimePoseSampleCount{0};
    uint32_t sceneRuntimeBoundPoseSampleCount{0};
    float sceneRuntimePoseAdapterInfluence{0.0f};
    float sceneRuntimePoseReadabilityBias{0.0f};
    std::string sceneRuntimePoseAdapterBrief{"runtime_only/0.00/0.00"};
    int surfaceWidth{0};
    int surfaceHeight{0};
    std::string actionName{"idle"};
    std::string reactiveActionName{"idle"};
    float actionIntensity{0.0f};
    float reactiveActionIntensity{0.0f};
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

ConfiguredMouseCompanionRendererBackendPreferenceDiagnostics
EvaluateConfiguredMouseCompanionRendererBackendPreferenceDiagnostics(
    const AppController::MouseCompanionRuntimeStatus& status);

MouseCompanionRealRendererPreviewDiagnostics
EvaluateMouseCompanionRealRendererPreviewDiagnostics(
    const AppController::MouseCompanionRuntimeStatus& status);

} // namespace mousefx
