#pragma once

#include <memory>
#include <string>
#include <vector>

#include "MouseFx/Core/Control/MouseCompanionPluginV1Types.h"

namespace mousefx {

struct PetVisualHostUpdate {
    ScreenPoint pt{};
    int actionCode{0};
    float actionIntensity{0.0f};
    float headTintAmount{0.0f};
};

struct PetVisualHostRendererBackendCatalogEntry {
    std::string name;
    int priority{0};
    bool available{false};
    std::string unavailableReason;
    std::vector<std::string> unmetRequirements;
};

struct PetVisualHostRendererRuntimeDiagnostics {
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
    std::string defaultLaneCandidate{"builtin"};
    std::string defaultLaneSource{"runtime_builtin_default"};
    std::string defaultLaneRolloutStatus{"stay_on_builtin"};
    std::string defaultLaneStyleIntent{"style_candidate:none"};
};

struct PetVisualHostDiagnostics {
    std::string preferredRendererBackendSource;
    std::string preferredRendererBackend;
    std::string selectedRendererBackend;
    std::string rendererBackendSelectionReason;
    std::string rendererBackendFailureReason;
    std::vector<std::string> availableRendererBackends;
    std::vector<std::string> unavailableRendererBackends;
    std::vector<PetVisualHostRendererBackendCatalogEntry> rendererBackendCatalog;
    PetVisualHostRendererRuntimeDiagnostics rendererRuntime;
};

class IPetVisualHost {
public:
    virtual ~IPetVisualHost() = default;

    virtual bool Start(const MouseCompanionPetRuntimeConfig& config) = 0;
    virtual void Shutdown() = 0;
    virtual bool Configure(const MouseCompanionPetRuntimeConfig& config) = 0;
    virtual bool Show() = 0;
    virtual void Hide() = 0;
    virtual bool LoadModel(const std::string& modelPath) = 0;
    virtual bool LoadActionLibrary(const std::string& actionLibraryPath) = 0;
    virtual bool LoadAppearanceProfile(const std::string& appearanceProfilePath) = 0;
    virtual bool ConfigurePoseBinding(const std::vector<std::string>& boneNames) = 0;
    virtual void MoveFollow(const ScreenPoint& pt) = 0;
    virtual void Update(const PetVisualHostUpdate& update) = 0;
    virtual void ApplyPose(const MouseCompanionPetPoseFrame& poseFrame) = 0;
    virtual bool IsActive() const = 0;
    virtual PetVisualHostDiagnostics ReadDiagnostics() const { return {}; }
};

} // namespace mousefx
