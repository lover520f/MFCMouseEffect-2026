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
    std::string sceneRuntimeModelAssetSourceState{"preview_only"};
    float sceneRuntimeModelAssetSourceReadiness{0.0f};
    std::string sceneRuntimeModelAssetSourceBrief{"preview_only/unknown/model:0/action:0/appearance:0"};
    std::string sceneRuntimeModelAssetSourcePathBrief{"model:-|action:-|appearance:default"};
    std::string sceneRuntimeModelAssetSourceValueBrief{"format:unknown|readiness:0.00"};
    std::string sceneRuntimeModelAssetManifestState{"preview_only"};
    uint32_t sceneRuntimeModelAssetManifestEntryCount{0};
    uint32_t sceneRuntimeModelAssetManifestResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetManifestBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetManifestEntryBrief{"model:-|action:-|appearance:default"};
    std::string sceneRuntimeModelAssetManifestValueBrief{"model:(0,0.00)|action:(0,0.00)|appearance:(0,0.00)"};
    std::string sceneRuntimeModelAssetCatalogState{"preview_only"};
    uint32_t sceneRuntimeModelAssetCatalogEntryCount{0};
    uint32_t sceneRuntimeModelAssetCatalogResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetCatalogBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetCatalogEntryBrief{"model:-|action:-|appearance:default"};
    std::string sceneRuntimeModelAssetCatalogValueBrief{"model:0.00|action:0.00|appearance:0.00"};
    std::string sceneRuntimeModelAssetBindingTableState{"preview_only"};
    uint32_t sceneRuntimeModelAssetBindingTableEntryCount{0};
    uint32_t sceneRuntimeModelAssetBindingTableResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetBindingTableBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetBindingTableSlotBrief{"model:-|action:-|appearance:-"};
    std::string sceneRuntimeModelAssetBindingTableValueBrief{"model:0.00|action:0.00|appearance:0.00"};
    std::string sceneRuntimeModelAssetRegistryState{"preview_only"};
    uint32_t sceneRuntimeModelAssetRegistryEntryCount{0};
    uint32_t sceneRuntimeModelAssetRegistryResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetRegistryAssetBrief{"model:-|slots:-|registry:-|binding:-"};
    std::string sceneRuntimeModelAssetRegistryValueBrief{"model:0.00|slots:0.00|registry:0.00|binding:0.00"};
    std::string sceneRuntimeModelAssetLoadState{"preview_only"};
    uint32_t sceneRuntimeModelAssetLoadEntryCount{0};
    uint32_t sceneRuntimeModelAssetLoadResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetLoadBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetLoadPlanBrief{"decode:-|actions:-|appearance:-|transforms:-|pose:runtime_only"};
    std::string sceneRuntimeModelAssetLoadValueBrief{"model:0.00|actions:0.00|appearance:0.00|transforms:0.00|pose:0.00"};
    std::string sceneRuntimeModelAssetDecodeState{"preview_only"};
    uint32_t sceneRuntimeModelAssetDecodeEntryCount{0};
    uint32_t sceneRuntimeModelAssetDecodeResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetDecodeBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetDecodePipelineBrief{
        "model:stub|action:stub|appearance:stub|transforms:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetDecodeValueBrief{
        "model:0.00|action:0.00|appearance:0.00|transforms:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetResidencyState{"preview_only"};
    uint32_t sceneRuntimeModelAssetResidencyEntryCount{0};
    uint32_t sceneRuntimeModelAssetResidencyResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetResidencyBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetResidencyCacheBrief{
        "model:cold|action:cold|appearance:cold|pose:cold|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetResidencyValueBrief{
        "model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetInstanceState{"preview_only"};
    uint32_t sceneRuntimeModelAssetInstanceEntryCount{0};
    uint32_t sceneRuntimeModelAssetInstanceResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetInstanceBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetInstanceSlotBrief{
        "model:stub|action:stub|appearance:stub|pose:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetInstanceValueBrief{
        "model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetActivationState{"preview_only"};
    uint32_t sceneRuntimeModelAssetActivationEntryCount{0};
    uint32_t sceneRuntimeModelAssetActivationResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetActivationBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetActivationRouteBrief{
        "action:idle|reactive:idle|follow:0|drag:0|hold:0|scroll:0|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetActivationValueBrief{
        "action:0.00|reactive:0.00|motion:0.00|pose:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetSessionState{"preview_only"};
    uint32_t sceneRuntimeModelAssetSessionEntryCount{0};
    uint32_t sceneRuntimeModelAssetSessionResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetSessionBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetSessionSessionBrief{
        "action:idle|reactive:idle|follow:0|drag:0|hold:0|scroll:0|pose:runtime_only"};
    std::string sceneRuntimeModelAssetSessionValueBrief{
        "session:0.00|motion:0.00|pose:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetBindReadyState{"preview_only"};
    uint32_t sceneRuntimeModelAssetBindReadyEntryCount{0};
    uint32_t sceneRuntimeModelAssetBindReadyResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetBindReadyBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetBindReadyBindingBrief{
        "binding:stub|pose:runtime_only|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetBindReadyValueBrief{
        "bind:0.00|pose:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetHandleState{"preview_only"};
    uint32_t sceneRuntimeModelAssetHandleEntryCount{0};
    uint32_t sceneRuntimeModelAssetHandleResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetHandleBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetHandleHandleBrief{
        "model:model_handle|action:action_handle|appearance:appearance_handle|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetHandleValueBrief{
        "model:0.00|action:0.00|appearance:0.00|adapter:0.00"};
    std::string sceneRuntimeModelSceneAdapterState{"preview_only"};
    float sceneRuntimeModelSceneSeamReadiness{0.0f};
    std::string sceneRuntimeModelSceneAdapterBrief{"preview_only/unknown/runtime_only"};
    std::string sceneRuntimeModelAssetSceneHookState{"preview_only"};
    uint32_t sceneRuntimeModelAssetSceneHookEntryCount{0};
    uint32_t sceneRuntimeModelAssetSceneHookResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetSceneHookBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetSceneHookHookBrief{
        "scene:stub|pose:stub|grounding:stub|overlay:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetSceneHookValueBrief{
        "scene:0.00|pose:0.00|grounding:0.00|overlay:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetSceneBindingState{"preview_only"};
    uint32_t sceneRuntimeModelAssetSceneBindingEntryCount{0};
    uint32_t sceneRuntimeModelAssetSceneBindingResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetSceneBindingBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetSceneBindingBindingBrief{
        "scene:stub|grounding:stub|overlay:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetSceneBindingValueBrief{
        "scene:0.00|grounding:0.00|overlay:0.00|adapter:0.00"};
    float sceneRuntimeModelNodeAdapterInfluence{0.0f};
    std::string sceneRuntimeModelNodeAdapterBrief{"preview_only/0.00"};
    std::string sceneRuntimeModelNodeChannelBrief{
        "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
    std::string sceneRuntimeModelAssetNodeAttachState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeAttachEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeAttachResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeAttachBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeAttachAttachBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeAttachValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeLiftState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeLiftEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeLiftResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeLiftBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeLiftLiftBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeLiftValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeBindState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeBindEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeBindResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeBindBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeBindBindBrief{
        "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeBindValueBrief{
        "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeResolveState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeResolveEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeResolveResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeResolveBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeResolveResolveBrief{
        "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeResolveValueBrief{
        "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"};
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
    std::string sceneRuntimeModelAssetNodeDriveState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeDriveEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeDriveResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeDriveBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeDriveDriveBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeDriveValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeMountState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeMountEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeMountResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeMountBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeMountMountBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeMountValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
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
    std::string sceneRuntimeModelAssetNodeRouteState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeRouteEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeRouteResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeRouteBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeRouteRouteBrief{
        "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeRouteValueBrief{
        "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeDispatchState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeDispatchEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeDispatchResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeDispatchBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeDispatchDispatchBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeDispatchValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeExecuteState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeExecuteEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeExecuteResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeExecuteBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeExecuteExecuteBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeExecuteValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeCommandState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeCommandEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeCommandResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeCommandBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeCommandCommandBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeCommandValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"};
    std::string sceneRuntimeModelAssetNodeControllerState{"preview_only"};
    uint32_t sceneRuntimeModelAssetNodeControllerEntryCount{0};
    uint32_t sceneRuntimeModelAssetNodeControllerResolvedEntryCount{0};
    std::string sceneRuntimeModelAssetNodeControllerBrief{"preview_only/0/0"};
    std::string sceneRuntimeModelAssetNodeControllerControllerBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"};
    std::string sceneRuntimeModelAssetNodeControllerValueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"};
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
    std::string sceneRuntimeAssetNodePoseChannelState{"preview_only"};
    uint32_t sceneRuntimeAssetNodePoseChannelEntryCount{0};
    uint32_t sceneRuntimeAssetNodePoseChannelResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodePoseChannelBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodePoseChannelNameBrief{
        "body:channel.body.posture|head:channel.head.expression|appendage:channel.appendage.motion|overlay:channel.overlay.fx|grounding:channel.grounding.shadow"};
    std::string sceneRuntimeAssetNodePoseChannelWeightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
    std::string sceneRuntimeAssetNodePoseConstraintState{"preview_only"};
    uint32_t sceneRuntimeAssetNodePoseConstraintEntryCount{0};
    uint32_t sceneRuntimeAssetNodePoseConstraintResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodePoseConstraintBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodePoseConstraintNameBrief{
        "body:constraint.body.posture|head:constraint.head.expression|appendage:constraint.appendage.motion|overlay:constraint.overlay.fx|grounding:constraint.grounding.shadow"};
    std::string sceneRuntimeAssetNodePoseConstraintValueBrief{
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)"};
    std::string sceneRuntimeAssetNodePoseSolveState{"preview_only"};
    uint32_t sceneRuntimeAssetNodePoseSolveEntryCount{0};
    uint32_t sceneRuntimeAssetNodePoseSolveResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodePoseSolveBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodePoseSolvePathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string sceneRuntimeAssetNodePoseSolveValueBrief{
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)"};
    std::string sceneRuntimeAssetNodeJointHintState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeJointHintEntryCount{0};
    uint32_t sceneRuntimeAssetNodeJointHintResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeJointHintBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeJointHintNameBrief{
        "body:joint.body.spine|head:joint.head.look|appendage:joint.appendage.reach|overlay:joint.overlay.fx|grounding:joint.grounding.balance"};
    std::string sceneRuntimeAssetNodeJointHintValueBrief{
        "body:(0.00,0.0,0.0,0.0)|head:(0.00,0.0,0.0,0.0)|appendage:(0.00,0.0,0.0,0.0)|overlay:(0.00,0.0,0.0,0.0)|grounding:(0.00,0.0,0.0,0.0)"};
    std::string sceneRuntimeAssetNodeArticulationState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeArticulationEntryCount{0};
    uint32_t sceneRuntimeAssetNodeArticulationResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeArticulationBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeArticulationNameBrief{
        "body:articulation.body.spine|head:articulation.head.look|appendage:articulation.appendage.reach|overlay:articulation.overlay.fx|grounding:articulation.grounding.balance"};
    std::string sceneRuntimeAssetNodeArticulationValueBrief{
        "body:(0.00,0.0,1.00,0.0)|head:(0.00,0.0,1.00,0.0)|appendage:(0.00,0.0,1.00,0.0)|overlay:(0.00,0.0,1.00,0.0)|grounding:(0.00,0.0,1.00,0.0)"};
    std::string sceneRuntimeAssetNodeLocalJointRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeLocalJointRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeLocalJointRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeLocalJointRegistryJointBrief{
        "body:local.body.spine|head:local.head.look|appendage:local.appendage.reach|overlay:local.overlay.fx|grounding:local.grounding.balance"};
    std::string sceneRuntimeAssetNodeLocalJointRegistryWeightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
    std::string sceneRuntimeAssetNodeArticulationMapState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeArticulationMapEntryCount{0};
    uint32_t sceneRuntimeAssetNodeArticulationMapResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeArticulationMapBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeArticulationMapNameBrief{
        "body:map.body.spine|head:map.head.look|appendage:map.appendage.reach|overlay:map.overlay.fx|grounding:map.grounding.balance"};
    std::string sceneRuntimeAssetNodeArticulationMapValueBrief{
        "body:(0.00,0.0,0.00)|head:(0.00,0.0,0.00)|appendage:(0.00,0.0,0.00)|overlay:(0.00,0.0,0.00)|grounding:(0.00,0.0,0.00)"};
    std::string sceneRuntimeAssetNodeControlRigHintState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeControlRigHintEntryCount{0};
    uint32_t sceneRuntimeAssetNodeControlRigHintResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeControlRigHintBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeControlRigHintNameBrief{
        "body:rig.body.spine|head:rig.head.look|appendage:rig.appendage.reach|overlay:rig.overlay.fx|grounding:rig.grounding.balance"};
    std::string sceneRuntimeAssetNodeControlRigHintValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeRigChannelState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeRigChannelEntryCount{0};
    uint32_t sceneRuntimeAssetNodeRigChannelResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeRigChannelBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeRigChannelNameBrief{
        "body:rig.channel.body.spine|head:rig.channel.head.look|appendage:rig.channel.appendage.reach|overlay:rig.channel.overlay.fx|grounding:rig.channel.grounding.balance"};
    std::string sceneRuntimeAssetNodeRigChannelValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeControlSurfaceState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeControlSurfaceEntryCount{0};
    uint32_t sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeControlSurfaceBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeControlSurfaceNameBrief{
        "body:surface.body.spine|head:surface.head.look|appendage:surface.appendage.reach|overlay:surface.overlay.fx|grounding:surface.grounding.balance"};
    std::string sceneRuntimeAssetNodeControlSurfaceValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeRigDriverState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeRigDriverEntryCount{0};
    uint32_t sceneRuntimeAssetNodeRigDriverResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeRigDriverBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeRigDriverNameBrief{
        "body:rig.driver.body.spine|head:rig.driver.head.look|appendage:rig.driver.appendage.reach|overlay:rig.driver.overlay.fx|grounding:rig.driver.grounding.balance"};
    std::string sceneRuntimeAssetNodeRigDriverValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceDriverState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceDriverEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceDriverBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceDriverNameBrief{
        "body:surface.driver.body.spine|head:surface.driver.head.look|appendage:surface.driver.appendage.reach|overlay:surface.driver.overlay.fx|grounding:surface.driver.grounding.balance"};
    std::string sceneRuntimeAssetNodeSurfaceDriverValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodePoseBusState{"preview_only"};
    uint32_t sceneRuntimeAssetNodePoseBusEntryCount{0};
    uint32_t sceneRuntimeAssetNodePoseBusResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodePoseBusBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodePoseBusNameBrief{
        "body:pose.bus.body.spine|head:pose.bus.head.look|appendage:pose.bus.appendage.reach|overlay:pose.bus.overlay.fx|grounding:pose.bus.grounding.balance"};
    std::string sceneRuntimeAssetNodePoseBusValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeControllerTableState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeControllerTableEntryCount{0};
    uint32_t sceneRuntimeAssetNodeControllerTableResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeControllerTableBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeControllerTableNameBrief{
        "body:controller.body.spine|head:controller.head.look|appendage:controller.appendage.reach|overlay:controller.overlay.fx|grounding:controller.grounding.balance"};
    std::string sceneRuntimeAssetNodeControllerTableValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeControllerRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeControllerRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeControllerRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeControllerRegistryNameBrief{
        "body:registry.body.spine|head:registry.head.look|appendage:registry.appendage.reach|overlay:registry.overlay.fx|grounding:registry.grounding.balance"};
    std::string sceneRuntimeAssetNodeControllerRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeDriverBusState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeDriverBusEntryCount{0};
    uint32_t sceneRuntimeAssetNodeDriverBusResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeDriverBusBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeDriverBusNameBrief{
        "body:driver.bus.body.spine|head:driver.bus.head.look|appendage:driver.bus.appendage.reach|overlay:driver.bus.overlay.fx|grounding:driver.bus.grounding.balance"};
    std::string sceneRuntimeAssetNodeDriverBusValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeControllerDriverRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeControllerDriverRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeControllerDriverRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeControllerDriverRegistryNameBrief{
        "body:controller.driver.body.spine|head:controller.driver.head.look|appendage:controller.driver.appendage.reach|overlay:controller.driver.overlay.fx|grounding:controller.driver.grounding.balance"};
    std::string sceneRuntimeAssetNodeControllerDriverRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionLaneState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionLaneEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionLaneBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionLaneNameBrief{
        "body:execution.lane.body.spine|head:execution.lane.head.look|appendage:execution.lane.appendage.reach|overlay:execution.lane.overlay.fx|grounding:execution.lane.grounding.balance"};
    std::string sceneRuntimeAssetNodeExecutionLaneValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeControllerPhaseState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeControllerPhaseEntryCount{0};
    uint32_t sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeControllerPhaseBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeControllerPhaseNameBrief{
        "body:controller.phase.body.spine|head:controller.phase.head.look|appendage:controller.phase.appendage.reach|overlay:controller.phase.overlay.fx|grounding:controller.phase.grounding.balance"};
    std::string sceneRuntimeAssetNodeControllerPhaseValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionSurfaceState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionSurfaceEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionSurfaceBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionSurfaceNameBrief{
        "body:execution.surface.body.shell|head:execution.surface.head.mask|appendage:execution.surface.appendage.trim|overlay:execution.surface.overlay.fx|grounding:execution.surface.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionSurfaceValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeControllerPhaseRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeControllerPhaseRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief{
        "body:phase.registry.body.spine|head:phase.registry.head.look|appendage:phase.registry.appendage.reach|overlay:phase.registry.overlay.fx|grounding:phase.registry.grounding.balance"};
    std::string sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceCompositionBusState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceCompositionBusBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief{
        "body:surface.bus.body.shell|head:surface.bus.head.mask|appendage:surface.bus.appendage.trim|overlay:surface.bus.overlay.fx|grounding:surface.bus.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionStackState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionStackEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionStackResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionStackBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionStackNameBrief{
        "body:execution.stack.body.shell|head:execution.stack.head.mask|appendage:execution.stack.appendage.trim|overlay:execution.stack.overlay.fx|grounding:execution.stack.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionStackValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionStackRouterState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionStackRouterEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionStackRouterBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionStackRouterNameBrief{
        "body:execution.stack.router.body.shell|head:execution.stack.router.head.mask|appendage:execution.stack.router.appendage.trim|overlay:execution.stack.router.overlay.fx|grounding:execution.stack.router.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionStackRouterValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionStackRouterRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief{
        "body:execution.stack.router.registry.body.shell|head:execution.stack.router.registry.head.mask|appendage:execution.stack.router.registry.appendage.trim|overlay:execution.stack.router.registry.overlay.fx|grounding:execution.stack.router.registry.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeCompositionRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeCompositionRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeCompositionRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeCompositionRegistryNameBrief{
        "body:composition.registry.body.shell|head:composition.registry.head.mask|appendage:composition.registry.appendage.trim|overlay:composition.registry.overlay.fx|grounding:composition.registry.grounding.base"};
    std::string sceneRuntimeAssetNodeCompositionRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceRouteState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceRouteBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceRouteNameBrief{
        "body:surface.route.body.shell|head:surface.route.head.mask|appendage:surface.route.appendage.trim|overlay:surface.route.overlay.fx|grounding:surface.route.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceRouteValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceRouteRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceRouteRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief{
        "body:surface.route.registry.body.shell|head:surface.route.registry.head.mask|appendage:surface.route.registry.appendage.trim|overlay:surface.route.registry.overlay.fx|grounding:surface.route.registry.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceRouteRouterBusState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief{
        "body:surface.route.router.bus.body.shell|head:surface.route.router.bus.head.mask|appendage:surface.route.router.bus.appendage.trim|overlay:surface.route.router.bus.overlay.fx|grounding:surface.route.router.bus.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief{
        "body:surface.route.bus.registry.body.shell|head:surface.route.bus.registry.head.mask|appendage:surface.route.bus.registry.appendage.trim|overlay:surface.route.bus.registry.overlay.fx|grounding:surface.route.bus.registry.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief{
        "body:surface.route.bus.driver.body.shell|head:surface.route.bus.driver.head.mask|appendage:surface.route.bus.driver.appendage.trim|overlay:surface.route.bus.driver.overlay.fx|grounding:surface.route.bus.driver.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief{
        "body:surface.route.bus.driver.registry.body.shell|head:surface.route.bus.driver.registry.head.mask|appendage:surface.route.bus.driver.registry.appendage.trim|overlay:surface.route.bus.driver.registry.overlay.fx|grounding:surface.route.bus.driver.registry.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount{0};
    uint32_t sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief{
        "body:surface.route.bus.driver.registry.router.body.shell|head:surface.route.bus.driver.registry.router.head.mask|appendage:surface.route.bus.driver.registry.router.appendage.trim|overlay:surface.route.bus.driver.registry.router.overlay.fx|grounding:surface.route.bus.driver.registry.router.grounding.base"};
    std::string sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionDriverTableState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionDriverTableEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionDriverTableBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionDriverTableNameBrief{
        "body:execution.driver.body.shell|head:execution.driver.head.mask|appendage:execution.driver.appendage.trim|overlay:execution.driver.overlay.fx|grounding:execution.driver.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionDriverTableValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterTableState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterTableBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief{
        "body:execution.driver.router.body.shell|head:execution.driver.router.head.mask|appendage:execution.driver.router.appendage.trim|overlay:execution.driver.router.overlay.fx|grounding:execution.driver.router.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief{
        "body:execution.driver.router.registry.body.shell|head:execution.driver.router.registry.head.mask|appendage:execution.driver.router.registry.appendage.trim|overlay:execution.driver.router.registry.overlay.fx|grounding:execution.driver.router.registry.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief{
        "body:execution.driver.router.registry.bus.body.shell|head:execution.driver.router.registry.bus.head.mask|appendage:execution.driver.router.registry.bus.appendage.trim|overlay:execution.driver.router.registry.bus.overlay.fx|grounding:execution.driver.router.registry.bus.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState{"preview_only"};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount{0};
    uint32_t sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount{0};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief{"preview_only/0/0"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief{
        "body:execution.driver.router.registry.bus.registry.body.shell|head:execution.driver.router.registry.bus.registry.head.mask|appendage:execution.driver.router.registry.bus.registry.appendage.trim|overlay:execution.driver.router.registry.bus.registry.overlay.fx|grounding:execution.driver.router.registry.bus.registry.grounding.base"};
    std::string sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
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
