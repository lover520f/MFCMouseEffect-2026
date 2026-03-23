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
    std::string sceneRuntimeAssetNodeTransformState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeTransformEntryCount{0};
    uint32_t sceneRuntimeAssetNodeTransformResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeTransformBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeTransformPathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string sceneRuntimeAssetNodeTransformValueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
    std::string sceneRuntimeAssetNodeAnchorState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeAnchorEntryCount{0};
    uint32_t sceneRuntimeAssetNodeAnchorResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeAnchorBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeAnchorPointBrief{
        "body:(0.0,0.0)|head:(0.0,0.0)|appendage:(0.0,0.0)|overlay:(0.0,0.0)|grounding:(0.0,0.0)"};
    std::string sceneRuntimeAssetNodeAnchorScaleBrief{
        "body:1.00|head:1.00|appendage:1.00|overlay:1.00|grounding:1.00"};
    std::string sceneRuntimeAssetNodeResolverState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeResolverEntryCount{0};
    uint32_t sceneRuntimeAssetNodeResolverResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeResolverBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeResolverParentBrief{
        "body:root|head:body|appendage:body|overlay:head|grounding:body"};
    std::string sceneRuntimeAssetNodeResolverValueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
    std::string sceneRuntimeAssetNodeParentSpaceState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeParentSpaceEntryCount{0};
    uint32_t sceneRuntimeAssetNodeParentSpaceResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeParentSpaceBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeParentSpaceParentBrief{
        "body:root|head:body|appendage:body|overlay:head|grounding:body"};
    std::string sceneRuntimeAssetNodeParentSpaceValueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
    std::string sceneRuntimeAssetNodeTargetState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeTargetEntryCount{0};
    uint32_t sceneRuntimeAssetNodeTargetResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeTargetBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeTargetKindBrief{
        "body:body_target|head:head_target|appendage:appendage_target|overlay:overlay_target|grounding:grounding_target"};
    std::string sceneRuntimeAssetNodeTargetValueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
    std::string sceneRuntimeAssetNodeTargetResolverState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeTargetResolverEntryCount{0};
    uint32_t sceneRuntimeAssetNodeTargetResolverResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeTargetResolverBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeTargetResolverPathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string sceneRuntimeAssetNodeTargetResolverValueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
    std::string sceneRuntimeAssetNodeWorldSpaceState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeWorldSpaceEntryCount{0};
    uint32_t sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeWorldSpaceBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeWorldSpacePathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string sceneRuntimeAssetNodeWorldSpaceValueBrief{
        "body:(0.0,0.0,1.00)|head:(0.0,0.0,1.00)|appendage:(0.0,0.0,1.00)|overlay:(0.0,0.0,1.00)|grounding:(0.0,0.0,1.00)"};
    std::string sceneRuntimeAssetNodePoseState{"preview_only"};
    uint32_t sceneRuntimeAssetNodePoseEntryCount{0};
    uint32_t sceneRuntimeAssetNodePoseResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodePoseBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodePosePathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string sceneRuntimeAssetNodePoseValueBrief{
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)"};
    std::string sceneRuntimeAssetNodePoseResolverState{"preview_only"};
    uint32_t sceneRuntimeAssetNodePoseResolverEntryCount{0};
    uint32_t sceneRuntimeAssetNodePoseResolverResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodePoseResolverBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodePoseResolverPathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string sceneRuntimeAssetNodePoseResolverValueBrief{
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)"};
    std::string sceneRuntimeAssetNodePoseRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodePoseRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodePoseRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodePoseRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodePoseRegistryNodeBrief{
        "body:pose.body.root|head:pose.head.anchor|appendage:pose.appendage.anchor|overlay:pose.overlay.anchor|grounding:pose.grounding.anchor"};
    std::string sceneRuntimeAssetNodePoseRegistryWeightBrief{
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
