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
