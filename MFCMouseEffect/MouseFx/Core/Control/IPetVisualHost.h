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
    std::string sceneRuntimeAdapterMode{"runtime_only"};
    uint32_t sceneRuntimePoseSampleCount{0};
    uint32_t sceneRuntimeBoundPoseSampleCount{0};
    std::string sceneRuntimeModelSceneAdapterState{"preview_only"};
    float sceneRuntimeModelSceneSeamReadiness{0.0f};
    std::string sceneRuntimeModelSceneAdapterBrief{"preview_only/unknown/runtime_only"};
    float sceneRuntimeModelNodeAdapterInfluence{0.0f};
    std::string sceneRuntimeModelNodeAdapterBrief{"preview_only/0.00"};
    std::string sceneRuntimeModelNodeChannelBrief{
        "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
    std::string sceneRuntimeModelNodeGraphState{"preview_only"};
    uint32_t sceneRuntimeModelNodeGraphNodeCount{0};
    uint32_t sceneRuntimeModelNodeGraphBoundNodeCount{0};
    std::string sceneRuntimeModelNodeGraphBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelNodeBindingState{"preview_only"};
    uint32_t sceneRuntimeModelNodeBindingEntryCount{0};
    uint32_t sceneRuntimeModelNodeBindingBoundEntryCount{0};
    std::string sceneRuntimeModelNodeBindingBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelNodeBindingWeightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
    std::string sceneRuntimeModelNodeSlotState{"preview_only"};
    uint32_t sceneRuntimeModelNodeSlotCount{0};
    uint32_t sceneRuntimeModelNodeReadySlotCount{0};
    std::string sceneRuntimeModelNodeSlotBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelNodeSlotNameBrief{
        "body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor"};
    std::string sceneRuntimeModelNodeRegistryState{"preview_only"};
    uint32_t sceneRuntimeModelNodeRegistryEntryCount{0};
    uint32_t sceneRuntimeModelNodeRegistryResolvedEntryCount{0};
    std::string sceneRuntimeModelNodeRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelNodeRegistryAssetNodeBrief{
        "body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor"};
    std::string sceneRuntimeModelNodeRegistryWeightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
    std::string sceneRuntimeAssetNodeBindingState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeBindingEntryCount{0};
    uint32_t sceneRuntimeAssetNodeBindingResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeBindingBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeBindingPathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string sceneRuntimeAssetNodeBindingWeightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
    float sceneRuntimePoseAdapterInfluence{0.0f};
    float sceneRuntimePoseReadabilityBias{0.0f};
    std::string sceneRuntimePoseAdapterBrief{"runtime_only/0.00/0.00"};
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
