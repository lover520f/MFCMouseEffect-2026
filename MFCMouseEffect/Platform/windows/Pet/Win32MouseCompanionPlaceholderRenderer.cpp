#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginContractLabels.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderRenderer.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPainter.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendRegistry.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPoseAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginHost.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderSceneBuilder.h"

#include <chrono>

namespace mousefx::windows {
namespace {

uint64_t ReadRendererRuntimeTickMs() {
    using Clock = std::chrono::steady_clock;
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now().time_since_epoch())
            .count());
}

} // namespace

bool Win32MouseCompanionPlaceholderRenderer::Start() {
    ready_ = true;
    lastErrorReason_.clear();
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_ = Win32MouseCompanionRendererBackendRuntimeDiagnostics{};
    runtimeDiagnostics_.backendName = "placeholder";
    runtimeDiagnostics_.ready = true;
    return true;
}

void Win32MouseCompanionPlaceholderRenderer::Shutdown() {
    ready_ = false;
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_.ready = false;
    runtimeDiagnostics_.renderedFrame = false;
}

bool Win32MouseCompanionPlaceholderRenderer::IsReady() const {
    return ready_;
}

std::string Win32MouseCompanionPlaceholderRenderer::LastErrorReason() const {
    return lastErrorReason_;
}

void Win32MouseCompanionPlaceholderRenderer::Render(
    const Win32MouseCompanionRendererInput& input,
    Gdiplus::Graphics* graphics,
    int width,
    int height) const {
    if (!ready_ || !graphics || width <= 0 || height <= 0) {
        return;
    }

    graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    const Win32MouseCompanionRendererRuntime runtime = BuildWin32MouseCompanionRendererRuntime(input);
    const Win32MouseCompanionPlaceholderScene scene =
        BuildWin32MouseCompanionPlaceholderScene(runtime, width, height);
    const auto pluginSelection = ResolveWin32MouseCompanionRenderPluginSelection();
    const Win32MouseCompanionPlaceholderPainter painter{};
    painter.Paint(scene, graphics, width, height);

    Win32MouseCompanionRendererBackendRuntimeDiagnostics diagnostics{};
    diagnostics.backendName = "placeholder";
    diagnostics.ready = ready_;
    diagnostics.renderedFrame = true;
    diagnostics.renderedFrameCount = 1;
    diagnostics.lastRenderTickMs = ReadRendererRuntimeTickMs();
    diagnostics.actionName = input.actionName;
    diagnostics.reactiveActionName = input.reactiveActionName;
    diagnostics.actionIntensity = input.actionIntensity;
    diagnostics.reactiveActionIntensity = input.reactiveActionIntensity;
    diagnostics.modelReady = input.modelAssetAvailable;
    diagnostics.actionLibraryReady = input.actionLibraryAvailable;
    diagnostics.appearanceProfileReady = true;
    diagnostics.poseFrameAvailable = input.poseFrameAvailable;
    diagnostics.poseBindingConfigured = input.poseBindingConfigured;
    diagnostics.sceneRuntimeAdapterMode = runtime.sceneRuntimeAdapterMode;
    diagnostics.sceneRuntimePoseSampleCount = runtime.sceneRuntimePoseSampleCount;
    diagnostics.sceneRuntimeBoundPoseSampleCount =
        runtime.sceneRuntimeBoundPoseSampleCount;
    const auto poseAdapterProfile =
        BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
            runtime.sceneRuntimeAdapterMode,
            runtime.sceneRuntimePoseSampleCount,
            runtime.sceneRuntimeBoundPoseSampleCount);
    diagnostics.sceneRuntimePoseAdapterInfluence = poseAdapterProfile.influence;
    diagnostics.sceneRuntimePoseReadabilityBias = poseAdapterProfile.readabilityBias;
    diagnostics.sceneRuntimePoseAdapterBrief = poseAdapterProfile.brief;
    diagnostics.facingDirection = input.facingDirection;
    diagnostics.surfaceWidth = width;
    diagnostics.surfaceHeight = height;
    diagnostics.modelSourceFormat =
        input.modelAssetAvailable && !input.modelPath.empty() ? "phase1_placeholder" : "unknown";
    Win32MouseCompanionRealRendererAssetResources modelResources{};
    modelResources.modelPath = input.modelPath;
    modelResources.modelSourceFormat = diagnostics.modelSourceFormat;
    modelResources.modelReady = input.modelAssetAvailable && !input.modelPath.empty();
    const auto modelSceneAdapterProfile =
        BuildWin32MouseCompanionRealRendererModelSceneAdapterProfile(
            modelResources,
            runtime.sceneRuntimeAdapterMode,
            runtime.poseFrameAvailable,
            runtime.poseBindingConfigured);
    diagnostics.sceneRuntimeModelSceneAdapterState =
        modelSceneAdapterProfile.seamState;
    diagnostics.sceneRuntimeModelSceneSeamReadiness =
        modelSceneAdapterProfile.seamReadiness;
    diagnostics.sceneRuntimeModelSceneAdapterBrief =
        modelSceneAdapterProfile.brief;
    diagnostics.sceneRuntimeModelAssetSourceState = "preview_only";
    diagnostics.sceneRuntimeModelAssetSourceReadiness = 0.0f;
    diagnostics.sceneRuntimeModelAssetSourceBrief =
        "preview_only/unknown/model:0/action:0/appearance:0";
    diagnostics.sceneRuntimeModelAssetSourcePathBrief =
        "model:-|action:-|appearance:default";
    diagnostics.sceneRuntimeModelAssetSourceValueBrief =
        "format:unknown|readiness:0.00";
    diagnostics.sceneRuntimeModelAssetManifestState = "preview_only";
    diagnostics.sceneRuntimeModelAssetManifestEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetManifestResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetManifestBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetManifestEntryBrief =
        "model:-|action:-|appearance:default";
    diagnostics.sceneRuntimeModelAssetManifestValueBrief =
        "model:(0,0.00)|action:(0,0.00)|appearance:(0,0.00)";
    diagnostics.sceneRuntimeModelAssetCatalogState = "preview_only";
    diagnostics.sceneRuntimeModelAssetCatalogEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetCatalogResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetCatalogBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetCatalogEntryBrief =
        "model:-|action:-|appearance:default";
    diagnostics.sceneRuntimeModelAssetCatalogValueBrief =
        "model:0.00|action:0.00|appearance:0.00";
    diagnostics.sceneRuntimeModelAssetBindingTableState = "preview_only";
    diagnostics.sceneRuntimeModelAssetBindingTableEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetBindingTableResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetBindingTableBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetBindingTableSlotBrief =
        "model:-|action:-|appearance:-";
    diagnostics.sceneRuntimeModelAssetBindingTableValueBrief =
        "model:0.00|action:0.00|appearance:0.00";
    diagnostics.sceneRuntimeModelAssetRegistryState = "preview_only";
    diagnostics.sceneRuntimeModelAssetRegistryEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetRegistryAssetBrief =
        "model:-|slots:-|registry:-|binding:-";
    diagnostics.sceneRuntimeModelAssetRegistryValueBrief =
        "model:0.00|slots:0.00|registry:0.00|binding:0.00";
    diagnostics.sceneRuntimeModelAssetLoadState = "preview_only";
    diagnostics.sceneRuntimeModelAssetLoadEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetLoadResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetLoadBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetLoadPlanBrief =
        "decode:-|actions:-|appearance:-|transforms:-|pose:runtime_only";
    diagnostics.sceneRuntimeModelAssetLoadValueBrief =
        "model:0.00|actions:0.00|appearance:0.00|transforms:0.00|pose:0.00";
    diagnostics.sceneRuntimeModelAssetDecodeState = "preview_only";
    diagnostics.sceneRuntimeModelAssetDecodeEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetDecodeResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetDecodeBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetDecodePipelineBrief =
        "model:stub|action:stub|appearance:stub|transforms:stub|adapter:runtime_only";
    diagnostics.sceneRuntimeModelAssetDecodeValueBrief =
        "model:0.00|action:0.00|appearance:0.00|transforms:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelAssetResidencyState = "preview_only";
    diagnostics.sceneRuntimeModelAssetResidencyEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetResidencyResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetResidencyBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetResidencyCacheBrief =
        "model:cold|action:cold|appearance:cold|pose:cold|adapter:runtime_only";
    diagnostics.sceneRuntimeModelAssetResidencyValueBrief =
        "model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelAssetInstanceState = "preview_only";
    diagnostics.sceneRuntimeModelAssetInstanceEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetInstanceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetInstanceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetInstanceSlotBrief =
        "model:stub|action:stub|appearance:stub|pose:stub|adapter:runtime_only";
    diagnostics.sceneRuntimeModelAssetInstanceValueBrief =
        "model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelAssetActivationState = "preview_only";
    diagnostics.sceneRuntimeModelAssetActivationEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetActivationResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetActivationBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetActivationRouteBrief =
        "action:idle|reactive:idle|follow:0|drag:0|hold:0|scroll:0|adapter:runtime_only";
    diagnostics.sceneRuntimeModelAssetActivationValueBrief =
        "action:0.00|reactive:0.00|motion:0.00|pose:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelAssetSessionState = "preview_only";
    diagnostics.sceneRuntimeModelAssetSessionEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetSessionResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetSessionBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetSessionSessionBrief =
        "action:idle|reactive:idle|follow:0|drag:0|hold:0|scroll:0|pose:runtime_only";
    diagnostics.sceneRuntimeModelAssetSessionValueBrief =
        "session:0.00|motion:0.00|pose:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelAssetBindReadyState = "preview_only";
    diagnostics.sceneRuntimeModelAssetBindReadyEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetBindReadyResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetBindReadyBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetBindReadyBindingBrief =
        "binding:stub|pose:runtime_only|adapter:runtime_only";
    diagnostics.sceneRuntimeModelAssetBindReadyValueBrief =
        "bind:0.00|pose:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelAssetHandleState = "preview_only";
    diagnostics.sceneRuntimeModelAssetHandleEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetHandleResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetHandleBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetHandleHandleBrief =
        "model:model_handle|action:action_handle|appearance:appearance_handle|adapter:runtime_only";
    diagnostics.sceneRuntimeModelAssetHandleValueBrief =
        "model:0.00|action:0.00|appearance:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelNodeAdapterInfluence = 0.0f;
    diagnostics.sceneRuntimeModelAssetSceneHookState = "preview_only";
    diagnostics.sceneRuntimeModelAssetSceneHookEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetSceneHookResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelAssetSceneHookBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelAssetSceneHookHookBrief =
        "scene:stub|pose:stub|grounding:stub|overlay:stub|adapter:runtime_only";
    diagnostics.sceneRuntimeModelAssetSceneHookValueBrief =
        "scene:0.00|pose:0.00|grounding:0.00|overlay:0.00|adapter:0.00";
    diagnostics.sceneRuntimeModelNodeAdapterBrief =
        modelSceneAdapterProfile.seamState + "/0.00";
    diagnostics.sceneRuntimeModelNodeChannelBrief =
        "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeModelNodeGraphState = "preview_only";
    diagnostics.sceneRuntimeModelNodeGraphNodeCount = 0;
    diagnostics.sceneRuntimeModelNodeGraphBoundNodeCount = 0;
    diagnostics.sceneRuntimeModelNodeGraphBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeBindingState = "preview_only";
    diagnostics.sceneRuntimeModelNodeBindingEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeBindingBoundEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeBindingBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeBindingWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeModelNodeSlotState = "preview_only";
    diagnostics.sceneRuntimeModelNodeSlotCount = 0;
    diagnostics.sceneRuntimeModelNodeReadySlotCount = 0;
    diagnostics.sceneRuntimeModelNodeSlotBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeSlotNameBrief =
        "body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor";
    diagnostics.sceneRuntimeModelNodeRegistryState = "preview_only";
    diagnostics.sceneRuntimeModelNodeRegistryEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeRegistryAssetNodeBrief =
        "body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor";
    diagnostics.sceneRuntimeModelNodeRegistryWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodeBindingState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeBindingEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeBindingResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeBindingBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeBindingPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeBindingWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodeTransformState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeTransformEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTransformResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTransformBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeTransformPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeTransformValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeAnchorState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeAnchorEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeAnchorResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeAnchorBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeAnchorPointBrief =
        "body:(0.0,0.0)|head:(0.0,0.0)|appendage:(0.0,0.0)|overlay:(0.0,0.0)|grounding:(0.0,0.0)";
    diagnostics.sceneRuntimeAssetNodeAnchorScaleBrief =
        "body:1.00|head:1.00|appendage:1.00|overlay:1.00|grounding:1.00";
    diagnostics.sceneRuntimeAssetNodeResolverState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeResolverEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeResolverResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeResolverBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeResolverParentBrief =
        "body:root|head:body|appendage:body|overlay:head|grounding:body";
    diagnostics.sceneRuntimeAssetNodeResolverValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeParentSpaceState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeParentSpaceEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeParentSpaceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeParentSpaceParentBrief =
        "body:root|head:body|appendage:body|overlay:head|grounding:body";
    diagnostics.sceneRuntimeAssetNodeParentSpaceValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeTargetState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeTargetEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeTargetKindBrief =
        "body:body_target|head:head_target|appendage:appendage_target|overlay:overlay_target|grounding:grounding_target";
    diagnostics.sceneRuntimeAssetNodeTargetValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeTargetResolverState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeTargetResolverEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetResolverBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeTargetResolverPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeTargetResolverValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeWorldSpaceState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeWorldSpaceEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeWorldSpacePathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeWorldSpaceValueBrief =
        "body:(0.0,0.0,1.00)|head:(0.0,0.0,1.00)|appendage:(0.0,0.0,1.00)|overlay:(0.0,0.0,1.00)|grounding:(0.0,0.0,1.00)";
    diagnostics.sceneRuntimeAssetNodePoseState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePosePathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodePoseValueBrief =
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodePoseResolverState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseResolverEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseResolverResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseResolverBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseResolverPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodePoseResolverValueBrief =
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodePoseRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseRegistryNodeBrief =
        "body:pose.body.root|head:pose.head.anchor|appendage:pose.appendage.anchor|overlay:pose.overlay.anchor|grounding:pose.grounding.anchor";
    diagnostics.sceneRuntimeAssetNodePoseRegistryWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodePoseChannelState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseChannelEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseChannelResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseChannelBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseChannelNameBrief =
        "body:channel.body.posture|head:channel.head.expression|appendage:channel.appendage.motion|overlay:channel.overlay.fx|grounding:channel.grounding.shadow";
    diagnostics.sceneRuntimeAssetNodePoseChannelWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodePoseConstraintState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseConstraintEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseConstraintBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseConstraintNameBrief =
        "body:constraint.body.posture|head:constraint.head.expression|appendage:constraint.appendage.motion|overlay:constraint.overlay.fx|grounding:constraint.grounding.shadow";
    diagnostics.sceneRuntimeAssetNodePoseConstraintValueBrief =
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodePoseSolveState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseSolveEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseSolveResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseSolveBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseSolvePathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodePoseSolveValueBrief =
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodeJointHintState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeJointHintEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeJointHintResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeJointHintBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeJointHintNameBrief =
        "body:joint.body.spine|head:joint.head.look|appendage:joint.appendage.reach|overlay:joint.overlay.fx|grounding:joint.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeJointHintValueBrief =
        "body:(0.00,0.0,0.0,0.0)|head:(0.00,0.0,0.0,0.0)|appendage:(0.00,0.0,0.0,0.0)|overlay:(0.00,0.0,0.0,0.0)|grounding:(0.00,0.0,0.0,0.0)";
    diagnostics.sceneRuntimeAssetNodeArticulationState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeArticulationEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeArticulationNameBrief =
        "body:articulation.body.spine|head:articulation.head.look|appendage:articulation.appendage.reach|overlay:articulation.overlay.fx|grounding:articulation.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeArticulationValueBrief =
        "body:(0.00,0.0,1.00,0.0)|head:(0.00,0.0,1.00,0.0)|appendage:(0.00,0.0,1.00,0.0)|overlay:(0.00,0.0,1.00,0.0)|grounding:(0.00,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryJointBrief =
        "body:local.body.spine|head:local.head.look|appendage:local.appendage.reach|overlay:local.overlay.fx|grounding:local.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodeArticulationMapState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeArticulationMapEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationMapBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeArticulationMapNameBrief =
        "body:map.body.spine|head:map.head.look|appendage:map.appendage.reach|overlay:map.overlay.fx|grounding:map.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeArticulationMapValueBrief =
        "body:(0.00,0.0,0.00)|head:(0.00,0.0,0.00)|appendage:(0.00,0.0,0.00)|overlay:(0.00,0.0,0.00)|grounding:(0.00,0.0,0.00)";
    diagnostics.sceneRuntimeAssetNodeControlRigHintState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControlRigHintEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlRigHintBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControlRigHintNameBrief =
        "body:rig.body.spine|head:rig.head.look|appendage:rig.appendage.reach|overlay:rig.overlay.fx|grounding:rig.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControlRigHintValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeRigChannelState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeRigChannelEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeRigChannelResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeRigChannelBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeRigChannelNameBrief =
        "body:rig.channel.body.spine|head:rig.channel.head.look|appendage:rig.channel.appendage.reach|overlay:rig.channel.overlay.fx|grounding:rig.channel.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeRigChannelValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceNameBrief =
        "body:surface.body.spine|head:surface.head.look|appendage:surface.appendage.reach|overlay:surface.overlay.fx|grounding:surface.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodePoseBusState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseBusEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseBusResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseBusBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseBusNameBrief =
        "body:pose.bus.body.spine|head:pose.bus.head.look|appendage:pose.bus.appendage.reach|overlay:pose.bus.overlay.fx|grounding:pose.bus.grounding.balance";
    diagnostics.sceneRuntimeAssetNodePoseBusValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControllerTableState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControllerTableEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerTableResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerTableBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControllerTableNameBrief =
        "body:controller.body.spine|head:controller.head.look|appendage:controller.appendage.reach|overlay:controller.overlay.fx|grounding:controller.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControllerTableValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControllerRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControllerRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControllerRegistryNameBrief =
        "body:registry.body.spine|head:registry.head.look|appendage:registry.appendage.reach|overlay:registry.overlay.fx|grounding:registry.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControllerRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeDriverBusState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeDriverBusEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeDriverBusResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeDriverBusBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeDriverBusNameBrief =
        "body:driver.bus.body.spine|head:driver.bus.head.look|appendage:driver.bus.appendage.reach|overlay:driver.bus.overlay.fx|grounding:driver.bus.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeDriverBusValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryNameBrief =
        "body:controller.driver.body.spine|head:controller.driver.head.look|appendage:controller.driver.appendage.reach|overlay:controller.driver.overlay.fx|grounding:controller.driver.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionLaneState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionLaneEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionLaneNameBrief =
        "body:execution.lane.body.spine|head:execution.lane.head.look|appendage:execution.lane.appendage.reach|overlay:execution.lane.overlay.fx|grounding:execution.lane.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeExecutionLaneValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseNameBrief =
        "body:controller.phase.body.spine|head:controller.phase.head.look|appendage:controller.phase.appendage.reach|overlay:controller.phase.overlay.fx|grounding:controller.phase.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceNameBrief =
        "body:execution.surface.body.shell|head:execution.surface.head.mask|appendage:execution.surface.appendage.trim|overlay:execution.surface.overlay.fx|grounding:execution.surface.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief =
        "body:phase.registry.body.spine|head:phase.registry.head.look|appendage:phase.registry.appendage.reach|overlay:phase.registry.overlay.fx|grounding:phase.registry.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief =
        "body:surface.bus.body.shell|head:surface.bus.head.mask|appendage:surface.bus.appendage.trim|overlay:surface.bus.overlay.fx|grounding:surface.bus.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionStackState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionStackEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionStackResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionStackBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionStackNameBrief =
        "body:execution.stack.body.shell|head:execution.stack.head.mask|appendage:execution.stack.appendage.trim|overlay:execution.stack.overlay.fx|grounding:execution.stack.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionStackValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterNameBrief =
        "body:execution.stack.router.body.shell|head:execution.stack.router.head.mask|appendage:execution.stack.router.appendage.trim|overlay:execution.stack.router.overlay.fx|grounding:execution.stack.router.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief =
        "body:execution.stack.router.registry.body.shell|head:execution.stack.router.registry.head.mask|appendage:execution.stack.router.registry.appendage.trim|overlay:execution.stack.router.registry.overlay.fx|grounding:execution.stack.router.registry.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryNameBrief =
        "body:composition.registry.body.shell|head:composition.registry.head.mask|appendage:composition.registry.appendage.trim|overlay:composition.registry.overlay.fx|grounding:composition.registry.grounding.base";
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteNameBrief =
        "body:surface.route.body.shell|head:surface.route.head.mask|appendage:surface.route.appendage.trim|overlay:surface.route.overlay.fx|grounding:surface.route.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief =
        "body:surface.route.registry.body.shell|head:surface.route.registry.head.mask|appendage:surface.route.registry.appendage.trim|overlay:surface.route.registry.overlay.fx|grounding:surface.route.registry.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief =
        "body:surface.route.router.bus.body.shell|head:surface.route.router.bus.head.mask|appendage:surface.route.router.bus.appendage.trim|overlay:surface.route.router.bus.overlay.fx|grounding:surface.route.router.bus.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief =
        "body:surface.route.bus.registry.body.shell|head:surface.route.bus.registry.head.mask|appendage:surface.route.bus.registry.appendage.trim|overlay:surface.route.bus.registry.overlay.fx|grounding:surface.route.bus.registry.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief =
        "body:surface.route.bus.driver.body.shell|head:surface.route.bus.driver.head.mask|appendage:surface.route.bus.driver.appendage.trim|overlay:surface.route.bus.driver.overlay.fx|grounding:surface.route.bus.driver.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief =
        "body:surface.route.bus.driver.registry.body.shell|head:surface.route.bus.driver.registry.head.mask|appendage:surface.route.bus.driver.registry.appendage.trim|overlay:surface.route.bus.driver.registry.overlay.fx|grounding:surface.route.bus.driver.registry.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief =
        "body:surface.route.bus.driver.registry.router.body.shell|head:surface.route.bus.driver.registry.router.head.mask|appendage:surface.route.bus.driver.registry.router.appendage.trim|overlay:surface.route.bus.driver.registry.router.overlay.fx|grounding:surface.route.bus.driver.registry.router.grounding.base";
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableNameBrief =
        "body:execution.driver.body.shell|head:execution.driver.head.mask|appendage:execution.driver.appendage.trim|overlay:execution.driver.overlay.fx|grounding:execution.driver.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief =
        "body:execution.driver.router.body.shell|head:execution.driver.router.head.mask|appendage:execution.driver.router.appendage.trim|overlay:execution.driver.router.overlay.fx|grounding:execution.driver.router.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief =
        "body:execution.driver.router.registry.body.shell|head:execution.driver.router.registry.head.mask|appendage:execution.driver.router.registry.appendage.trim|overlay:execution.driver.router.registry.overlay.fx|grounding:execution.driver.router.registry.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief =
        "body:execution.driver.router.registry.bus.body.shell|head:execution.driver.router.registry.bus.head.mask|appendage:execution.driver.router.registry.bus.appendage.trim|overlay:execution.driver.router.registry.bus.overlay.fx|grounding:execution.driver.router.registry.bus.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief =
        "body:execution.driver.router.registry.bus.registry.body.shell|head:execution.driver.router.registry.bus.registry.head.mask|appendage:execution.driver.router.registry.bus.registry.appendage.trim|overlay:execution.driver.router.registry.bus.registry.overlay.fx|grounding:execution.driver.router.registry.bus.registry.grounding.base";
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.appearanceSkinVariantId = input.appearanceProfile.skinVariantId;
    diagnostics.appearanceAccessoryIds = input.appearanceProfile.enabledAccessoryIds;
    const auto accessoryFamily =
        ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
            input.appearanceProfile.enabledAccessoryIds);
    diagnostics.appearanceAccessoryFamily =
        ToStringWin32MouseCompanionRealRendererAppearanceAccessoryFamily(accessoryFamily);
    diagnostics.appearanceComboPreset =
        ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
            ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
                input.appearanceProfile.skinVariantId,
                accessoryFamily));
    if (pluginSelection.comboPresetOverride !=
        Win32MouseCompanionRealRendererAppearanceComboPreset::None) {
        diagnostics.appearanceComboPreset =
            ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
                pluginSelection.comboPresetOverride);
    }
    diagnostics.appearanceRequestedPresetId = input.appearanceProfile.requestedPresetId;
    diagnostics.appearanceResolvedPresetId = input.appearanceProfile.resolvedPresetId;
    diagnostics.appearancePluginId = pluginSelection.pluginId;
    diagnostics.appearancePluginKind = pluginSelection.pluginKind;
    diagnostics.appearancePluginSource = pluginSelection.pluginSource;
    diagnostics.appearancePluginSelectionReason = pluginSelection.selectionReason;
    diagnostics.appearancePluginFailureReason = pluginSelection.failureReason;
    diagnostics.appearancePluginManifestPath = pluginSelection.manifestPath;
    diagnostics.appearancePluginRuntimeBackend = pluginSelection.runtimeBackend;
    diagnostics.appearancePluginMetadataPath = pluginSelection.metadataPath;
    diagnostics.appearancePluginMetadataSchemaVersion =
        pluginSelection.metadataSchemaVersion;
    diagnostics.appearancePluginAppearanceSemanticsMode =
        pluginSelection.appearanceSemanticsMode;
    diagnostics.appearancePluginSampleTier = pluginSelection.declaredSampleTier;
    diagnostics.defaultLaneCandidate = pluginSelection.defaultLaneCandidate;
    diagnostics.defaultLaneSource = pluginSelection.defaultLaneSource;
    diagnostics.defaultLaneRolloutStatus = pluginSelection.defaultLaneRolloutStatus;
    diagnostics.defaultLaneStyleIntent = pluginSelection.defaultLaneStyleIntent;
    diagnostics.defaultLaneCandidateTier = pluginSelection.defaultLaneCandidateTier;
    diagnostics.appearancePluginContractBrief =
        BuildWin32MouseCompanionRenderPluginContractBrief(
            diagnostics.appearancePluginAppearanceSemanticsMode,
            diagnostics.defaultLaneStyleIntent,
            diagnostics.appearancePluginSampleTier);
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    diagnostics.renderedFrameCount = runtimeDiagnostics_.renderedFrameCount + 1;
    runtimeDiagnostics_ = std::move(diagnostics);
}

Win32MouseCompanionRendererBackendRuntimeDiagnostics
Win32MouseCompanionPlaceholderRenderer::ReadRuntimeDiagnostics() const {
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    return runtimeDiagnostics_;
}

void RegisterWin32MouseCompanionPlaceholderRendererBackend() {
    static Win32MouseCompanionRendererBackendRegistrar<Win32MouseCompanionPlaceholderRenderer> registrar("placeholder", 100);
    (void)registrar;
}

} // namespace mousefx::windows
