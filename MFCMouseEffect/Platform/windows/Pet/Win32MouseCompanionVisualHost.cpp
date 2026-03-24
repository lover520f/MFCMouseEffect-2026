#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionVisualHost.h"

#include "MouseFx/Utils/StringUtils.h"
#include "MouseFx/Utils/TimeUtils.h"
#include "Platform/windows/Pet/Win32MouseCompanionActionRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendRegistry.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendPreferenceRequestBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionVisualRuntime.h"

namespace mousefx::windows {

bool Win32MouseCompanionVisualHost::Start(const MouseCompanionPetRuntimeConfig& config) {
    actionLibrary_ = {};
    actionRuntime_ = {};
    window_.SetRendererBackendPreferenceRequest(BuildWin32MouseCompanionRendererBackendPreferenceRequest(config));
    ResetWin32MouseCompanionVisualState(&state_, config, config.enabled && window_.Create());
    return state_.active;
}

void Win32MouseCompanionVisualHost::Shutdown() {
    window_.Shutdown();
    state_ = {};
    actionLibrary_ = {};
    actionRuntime_ = {};
    configuredBoneNames_.clear();
}

bool Win32MouseCompanionVisualHost::Configure(const MouseCompanionPetRuntimeConfig& config) {
    window_.SetRendererBackendPreferenceRequest(BuildWin32MouseCompanionRendererBackendPreferenceRequest(config));
    ApplyWin32MouseCompanionVisualConfig(&state_, config);
    if (!state_.active) {
        state_.visible = false;
        window_.Hide();
    } else {
        SyncWindow();
    }
    return state_.active;
}

bool Win32MouseCompanionVisualHost::Show() {
    if (!state_.active) {
        return false;
    }
    state_.visible = window_.Show();
    SyncWindow();
    return state_.visible;
}

void Win32MouseCompanionVisualHost::Hide() {
    window_.Hide();
    state_.visible = false;
}

bool Win32MouseCompanionVisualHost::LoadModel(const std::string& modelPath) {
    const std::string trimmed = TrimAscii(modelPath);
    if (!state_.active || trimmed.empty()) {
        return false;
    }
    ApplyWin32MouseCompanionModelAssetState(&state_, trimmed);
    SyncWindow();
    return false;
}

bool Win32MouseCompanionVisualHost::LoadActionLibrary(const std::string& actionLibraryPath) {
    const std::string trimmed = TrimAscii(actionLibraryPath);
    if (!state_.active || trimmed.empty()) {
        return false;
    }
    LoadWin32MouseCompanionActionRuntimeLibrary(&state_, &actionLibrary_, trimmed);
    RefreshActionRuntime();
    SyncWindow();
    return state_.actionLibraryAvailable;
}

bool Win32MouseCompanionVisualHost::LoadAppearanceProfile(const std::string& appearanceProfilePath) {
    const std::string trimmed = TrimAscii(appearanceProfilePath);
    if (!state_.active || trimmed.empty()) {
        return false;
    }
    Win32MouseCompanionAppearanceProfile profile{};
    const bool loaded = LoadWin32MouseCompanionAppearanceProfileFromPath(trimmed, &profile);
    ApplyWin32MouseCompanionAppearanceState(&state_, std::move(profile), loaded);
    SyncWindow();
    return loaded;
}

bool Win32MouseCompanionVisualHost::ConfigurePoseBinding(const std::vector<std::string>& boneNames) {
    configuredBoneNames_ = boneNames;
    ApplyWin32MouseCompanionPoseBindingState(&state_, !configuredBoneNames_.empty());
    SyncWindow();
    return false;
}

void Win32MouseCompanionVisualHost::MoveFollow(const ScreenPoint& pt) {
    ApplyWin32MouseCompanionFollowPoint(&state_, pt);
    SyncWindow();
}

void Win32MouseCompanionVisualHost::Update(const PetVisualHostUpdate& update) {
    ApplyWin32MouseCompanionHostUpdate(&state_, update);
    if (!state_.active) {
        return;
    }
    UpdateActionRuntimeSelection(update.actionCode == 2 || update.actionCode == 5);
    RefreshActionRuntime();
    SyncWindow();
}

void Win32MouseCompanionVisualHost::ApplyPose(const MouseCompanionPetPoseFrame& poseFrame) {
    ApplyWin32MouseCompanionPoseFrame(&state_, poseFrame);
    if (!state_.active) {
        return;
    }
    UpdateActionRuntimeSelection(false);
    RefreshActionRuntime();
    SyncWindow();
}

bool Win32MouseCompanionVisualHost::IsActive() const {
    return state_.active;
}

PetVisualHostDiagnostics Win32MouseCompanionVisualHost::ReadDiagnostics() const {
    PetVisualHostDiagnostics diagnostics{};
    diagnostics.preferredRendererBackendSource = window_.PreferredRendererBackendSource();
    diagnostics.preferredRendererBackend = window_.PreferredRendererBackendName();
    diagnostics.selectedRendererBackend = window_.SelectedRendererBackendName();
    diagnostics.rendererBackendSelectionReason = window_.RendererBackendSelectionReason();
    diagnostics.rendererBackendFailureReason = window_.RendererBackendFailureReason();
    diagnostics.availableRendererBackends = window_.AvailableRendererBackendNames();
    diagnostics.unavailableRendererBackends = window_.UnavailableRendererBackendNames();
    const auto rendererRuntime = window_.RendererRuntimeDiagnostics();
    diagnostics.rendererRuntime.backendName = rendererRuntime.backendName;
    diagnostics.rendererRuntime.ready = rendererRuntime.ready;
    diagnostics.rendererRuntime.renderedFrame = rendererRuntime.renderedFrame;
    diagnostics.rendererRuntime.renderedFrameCount = rendererRuntime.renderedFrameCount;
    diagnostics.rendererRuntime.lastRenderTickMs = rendererRuntime.lastRenderTickMs;
    diagnostics.rendererRuntime.actionName = rendererRuntime.actionName;
    diagnostics.rendererRuntime.reactiveActionName = rendererRuntime.reactiveActionName;
    diagnostics.rendererRuntime.actionIntensity = rendererRuntime.actionIntensity;
    diagnostics.rendererRuntime.reactiveActionIntensity = rendererRuntime.reactiveActionIntensity;
    diagnostics.rendererRuntime.modelReady = rendererRuntime.modelReady;
    diagnostics.rendererRuntime.actionLibraryReady = rendererRuntime.actionLibraryReady;
    diagnostics.rendererRuntime.appearanceProfileReady = rendererRuntime.appearanceProfileReady;
    diagnostics.rendererRuntime.poseFrameAvailable = rendererRuntime.poseFrameAvailable;
    diagnostics.rendererRuntime.poseBindingConfigured = rendererRuntime.poseBindingConfigured;
    diagnostics.rendererRuntime.sceneRuntimeAdapterMode =
        rendererRuntime.sceneRuntimeAdapterMode;
    diagnostics.rendererRuntime.sceneRuntimePoseSampleCount =
        rendererRuntime.sceneRuntimePoseSampleCount;
    diagnostics.rendererRuntime.sceneRuntimeBoundPoseSampleCount =
        rendererRuntime.sceneRuntimeBoundPoseSampleCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSourceState =
        rendererRuntime.sceneRuntimeModelAssetSourceState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSourceReadiness =
        rendererRuntime.sceneRuntimeModelAssetSourceReadiness;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSourceBrief =
        rendererRuntime.sceneRuntimeModelAssetSourceBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSourcePathBrief =
        rendererRuntime.sceneRuntimeModelAssetSourcePathBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSourceValueBrief =
        rendererRuntime.sceneRuntimeModelAssetSourceValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetManifestState =
        rendererRuntime.sceneRuntimeModelAssetManifestState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetManifestEntryCount =
        rendererRuntime.sceneRuntimeModelAssetManifestEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetManifestResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetManifestResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetManifestBrief =
        rendererRuntime.sceneRuntimeModelAssetManifestBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetManifestEntryBrief =
        rendererRuntime.sceneRuntimeModelAssetManifestEntryBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetManifestValueBrief =
        rendererRuntime.sceneRuntimeModelAssetManifestValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetCatalogState =
        rendererRuntime.sceneRuntimeModelAssetCatalogState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetCatalogEntryCount =
        rendererRuntime.sceneRuntimeModelAssetCatalogEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetCatalogResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetCatalogResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetCatalogBrief =
        rendererRuntime.sceneRuntimeModelAssetCatalogBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetCatalogEntryBrief =
        rendererRuntime.sceneRuntimeModelAssetCatalogEntryBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetCatalogValueBrief =
        rendererRuntime.sceneRuntimeModelAssetCatalogValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindingTableState =
        rendererRuntime.sceneRuntimeModelAssetBindingTableState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindingTableEntryCount =
        rendererRuntime.sceneRuntimeModelAssetBindingTableEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindingTableResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetBindingTableResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindingTableBrief =
        rendererRuntime.sceneRuntimeModelAssetBindingTableBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindingTableSlotBrief =
        rendererRuntime.sceneRuntimeModelAssetBindingTableSlotBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindingTableValueBrief =
        rendererRuntime.sceneRuntimeModelAssetBindingTableValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetRegistryState =
        rendererRuntime.sceneRuntimeModelAssetRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetRegistryEntryCount =
        rendererRuntime.sceneRuntimeModelAssetRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetRegistryBrief =
        rendererRuntime.sceneRuntimeModelAssetRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetRegistryAssetBrief =
        rendererRuntime.sceneRuntimeModelAssetRegistryAssetBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetRegistryValueBrief =
        rendererRuntime.sceneRuntimeModelAssetRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetLoadState =
        rendererRuntime.sceneRuntimeModelAssetLoadState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetLoadEntryCount =
        rendererRuntime.sceneRuntimeModelAssetLoadEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetLoadResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetLoadResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetLoadBrief =
        rendererRuntime.sceneRuntimeModelAssetLoadBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetLoadPlanBrief =
        rendererRuntime.sceneRuntimeModelAssetLoadPlanBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetLoadValueBrief =
        rendererRuntime.sceneRuntimeModelAssetLoadValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetDecodeState =
        rendererRuntime.sceneRuntimeModelAssetDecodeState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetDecodeEntryCount =
        rendererRuntime.sceneRuntimeModelAssetDecodeEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetDecodeResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetDecodeResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetDecodeBrief =
        rendererRuntime.sceneRuntimeModelAssetDecodeBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetDecodePipelineBrief =
        rendererRuntime.sceneRuntimeModelAssetDecodePipelineBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetDecodeValueBrief =
        rendererRuntime.sceneRuntimeModelAssetDecodeValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetResidencyState =
        rendererRuntime.sceneRuntimeModelAssetResidencyState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetResidencyEntryCount =
        rendererRuntime.sceneRuntimeModelAssetResidencyEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetResidencyResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetResidencyResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetResidencyBrief =
        rendererRuntime.sceneRuntimeModelAssetResidencyBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetResidencyCacheBrief =
        rendererRuntime.sceneRuntimeModelAssetResidencyCacheBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetResidencyValueBrief =
        rendererRuntime.sceneRuntimeModelAssetResidencyValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetInstanceState =
        rendererRuntime.sceneRuntimeModelAssetInstanceState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetInstanceEntryCount =
        rendererRuntime.sceneRuntimeModelAssetInstanceEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetInstanceResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetInstanceResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetInstanceBrief =
        rendererRuntime.sceneRuntimeModelAssetInstanceBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetInstanceSlotBrief =
        rendererRuntime.sceneRuntimeModelAssetInstanceSlotBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetInstanceValueBrief =
        rendererRuntime.sceneRuntimeModelAssetInstanceValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetActivationState =
        rendererRuntime.sceneRuntimeModelAssetActivationState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetActivationEntryCount =
        rendererRuntime.sceneRuntimeModelAssetActivationEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetActivationResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetActivationResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetActivationBrief =
        rendererRuntime.sceneRuntimeModelAssetActivationBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetActivationRouteBrief =
        rendererRuntime.sceneRuntimeModelAssetActivationRouteBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetActivationValueBrief =
        rendererRuntime.sceneRuntimeModelAssetActivationValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSessionState =
        rendererRuntime.sceneRuntimeModelAssetSessionState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSessionEntryCount =
        rendererRuntime.sceneRuntimeModelAssetSessionEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSessionResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetSessionResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSessionBrief =
        rendererRuntime.sceneRuntimeModelAssetSessionBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSessionSessionBrief =
        rendererRuntime.sceneRuntimeModelAssetSessionSessionBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSessionValueBrief =
        rendererRuntime.sceneRuntimeModelAssetSessionValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindReadyState =
        rendererRuntime.sceneRuntimeModelAssetBindReadyState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindReadyEntryCount =
        rendererRuntime.sceneRuntimeModelAssetBindReadyEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindReadyResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetBindReadyResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindReadyBrief =
        rendererRuntime.sceneRuntimeModelAssetBindReadyBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindReadyBindingBrief =
        rendererRuntime.sceneRuntimeModelAssetBindReadyBindingBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetBindReadyValueBrief =
        rendererRuntime.sceneRuntimeModelAssetBindReadyValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetHandleState =
        rendererRuntime.sceneRuntimeModelAssetHandleState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetHandleEntryCount =
        rendererRuntime.sceneRuntimeModelAssetHandleEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetHandleResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetHandleResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetHandleBrief =
        rendererRuntime.sceneRuntimeModelAssetHandleBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetHandleHandleBrief =
        rendererRuntime.sceneRuntimeModelAssetHandleHandleBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetHandleValueBrief =
        rendererRuntime.sceneRuntimeModelAssetHandleValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelSceneAdapterState =
        rendererRuntime.sceneRuntimeModelSceneAdapterState;
    diagnostics.rendererRuntime.sceneRuntimeModelSceneSeamReadiness =
        rendererRuntime.sceneRuntimeModelSceneSeamReadiness;
    diagnostics.rendererRuntime.sceneRuntimeModelSceneAdapterBrief =
        rendererRuntime.sceneRuntimeModelSceneAdapterBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneHookState =
        rendererRuntime.sceneRuntimeModelAssetSceneHookState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneHookEntryCount =
        rendererRuntime.sceneRuntimeModelAssetSceneHookEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneHookResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetSceneHookResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneHookBrief =
        rendererRuntime.sceneRuntimeModelAssetSceneHookBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneHookHookBrief =
        rendererRuntime.sceneRuntimeModelAssetSceneHookHookBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneHookValueBrief =
        rendererRuntime.sceneRuntimeModelAssetSceneHookValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneBindingState =
        rendererRuntime.sceneRuntimeModelAssetSceneBindingState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneBindingEntryCount =
        rendererRuntime.sceneRuntimeModelAssetSceneBindingEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneBindingResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetSceneBindingResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneBindingBrief =
        rendererRuntime.sceneRuntimeModelAssetSceneBindingBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneBindingBindingBrief =
        rendererRuntime.sceneRuntimeModelAssetSceneBindingBindingBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetSceneBindingValueBrief =
        rendererRuntime.sceneRuntimeModelAssetSceneBindingValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeAdapterInfluence =
        rendererRuntime.sceneRuntimeModelNodeAdapterInfluence;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeAdapterBrief =
        rendererRuntime.sceneRuntimeModelNodeAdapterBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeChannelBrief =
        rendererRuntime.sceneRuntimeModelNodeChannelBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeAttachState =
        rendererRuntime.sceneRuntimeModelAssetNodeAttachState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeAttachEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeAttachEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeAttachResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeAttachResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeAttachBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeAttachBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeAttachAttachBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeAttachAttachBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeAttachValueBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeAttachValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeLiftState =
        rendererRuntime.sceneRuntimeModelAssetNodeLiftState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeLiftEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeLiftEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeLiftResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeLiftResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeLiftBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeLiftBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeLiftLiftBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeLiftLiftBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeLiftValueBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeLiftValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeBindState =
        rendererRuntime.sceneRuntimeModelAssetNodeBindState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeBindEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeBindEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeBindResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeBindResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeBindBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeBindBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeBindBindBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeBindBindBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeBindValueBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeBindValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeResolveState =
        rendererRuntime.sceneRuntimeModelAssetNodeResolveState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeResolveEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeResolveEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeResolveResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeResolveResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeResolveBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeResolveBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeResolveResolveBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeResolveResolveBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeResolveValueBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeResolveValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphState =
        rendererRuntime.sceneRuntimeModelNodeGraphState;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphNodeCount =
        rendererRuntime.sceneRuntimeModelNodeGraphNodeCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphBoundNodeCount =
        rendererRuntime.sceneRuntimeModelNodeGraphBoundNodeCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphBrief =
        rendererRuntime.sceneRuntimeModelNodeGraphBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingState =
        rendererRuntime.sceneRuntimeModelNodeBindingState;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingEntryCount =
        rendererRuntime.sceneRuntimeModelNodeBindingEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingBoundEntryCount =
        rendererRuntime.sceneRuntimeModelNodeBindingBoundEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingBrief =
        rendererRuntime.sceneRuntimeModelNodeBindingBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingWeightBrief =
        rendererRuntime.sceneRuntimeModelNodeBindingWeightBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeDriveState =
        rendererRuntime.sceneRuntimeModelAssetNodeDriveState;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeDriveEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeDriveEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeDriveResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelAssetNodeDriveResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeDriveBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeDriveBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeDriveDriveBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeDriveDriveBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelAssetNodeDriveValueBrief =
        rendererRuntime.sceneRuntimeModelAssetNodeDriveValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotState =
        rendererRuntime.sceneRuntimeModelNodeSlotState;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotCount =
        rendererRuntime.sceneRuntimeModelNodeSlotCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeReadySlotCount =
        rendererRuntime.sceneRuntimeModelNodeReadySlotCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotBrief =
        rendererRuntime.sceneRuntimeModelNodeSlotBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotNameBrief =
        rendererRuntime.sceneRuntimeModelNodeSlotNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryState =
        rendererRuntime.sceneRuntimeModelNodeRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryEntryCount =
        rendererRuntime.sceneRuntimeModelNodeRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeModelNodeRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryBrief =
        rendererRuntime.sceneRuntimeModelNodeRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryAssetNodeBrief =
        rendererRuntime.sceneRuntimeModelNodeRegistryAssetNodeBrief;
    diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryWeightBrief =
        rendererRuntime.sceneRuntimeModelNodeRegistryWeightBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingState =
        rendererRuntime.sceneRuntimeAssetNodeBindingState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeBindingEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeBindingResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingBrief =
        rendererRuntime.sceneRuntimeAssetNodeBindingBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingPathBrief =
        rendererRuntime.sceneRuntimeAssetNodeBindingPathBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingWeightBrief =
        rendererRuntime.sceneRuntimeAssetNodeBindingWeightBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformState =
        rendererRuntime.sceneRuntimeAssetNodeTransformState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeTransformEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeTransformResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformBrief =
        rendererRuntime.sceneRuntimeAssetNodeTransformBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformPathBrief =
        rendererRuntime.sceneRuntimeAssetNodeTransformPathBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeTransformValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorState =
        rendererRuntime.sceneRuntimeAssetNodeAnchorState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeAnchorEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeAnchorResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorBrief =
        rendererRuntime.sceneRuntimeAssetNodeAnchorBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorPointBrief =
        rendererRuntime.sceneRuntimeAssetNodeAnchorPointBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorScaleBrief =
        rendererRuntime.sceneRuntimeAssetNodeAnchorScaleBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverState =
        rendererRuntime.sceneRuntimeAssetNodeResolverState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeResolverEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeResolverResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverBrief =
        rendererRuntime.sceneRuntimeAssetNodeResolverBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverParentBrief =
        rendererRuntime.sceneRuntimeAssetNodeResolverParentBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeResolverValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceState =
        rendererRuntime.sceneRuntimeAssetNodeParentSpaceState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeParentSpaceEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceBrief =
        rendererRuntime.sceneRuntimeAssetNodeParentSpaceBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceParentBrief =
        rendererRuntime.sceneRuntimeAssetNodeParentSpaceParentBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeParentSpaceValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetState =
        rendererRuntime.sceneRuntimeAssetNodeTargetState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeTargetEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeTargetResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetBrief =
        rendererRuntime.sceneRuntimeAssetNodeTargetBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetKindBrief =
        rendererRuntime.sceneRuntimeAssetNodeTargetKindBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeTargetValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverState =
        rendererRuntime.sceneRuntimeAssetNodeTargetResolverState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeTargetResolverEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverBrief =
        rendererRuntime.sceneRuntimeAssetNodeTargetResolverBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverPathBrief =
        rendererRuntime.sceneRuntimeAssetNodeTargetResolverPathBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeTargetResolverValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceState =
        rendererRuntime.sceneRuntimeAssetNodeWorldSpaceState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeWorldSpaceEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceBrief =
        rendererRuntime.sceneRuntimeAssetNodeWorldSpaceBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpacePathBrief =
        rendererRuntime.sceneRuntimeAssetNodeWorldSpacePathBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeWorldSpaceValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseState =
        rendererRuntime.sceneRuntimeAssetNodePoseState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePosePathBrief =
        rendererRuntime.sceneRuntimeAssetNodePosePathBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseValueBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverState =
        rendererRuntime.sceneRuntimeAssetNodePoseResolverState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseResolverEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseResolverResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseResolverBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverPathBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseResolverPathBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverValueBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseResolverValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryState =
        rendererRuntime.sceneRuntimeAssetNodePoseRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryNodeBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseRegistryNodeBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryWeightBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseRegistryWeightBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelState =
        rendererRuntime.sceneRuntimeAssetNodePoseChannelState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseChannelEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseChannelResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseChannelBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelNameBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseChannelNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelWeightBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseChannelWeightBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintState =
        rendererRuntime.sceneRuntimeAssetNodePoseConstraintState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseConstraintEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseConstraintBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintNameBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseConstraintNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintValueBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseConstraintValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveState =
        rendererRuntime.sceneRuntimeAssetNodePoseSolveState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseSolveEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseSolveResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseSolveBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolvePathBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseSolvePathBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveValueBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseSolveValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintState =
        rendererRuntime.sceneRuntimeAssetNodeJointHintState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeJointHintEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeJointHintResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintBrief =
        rendererRuntime.sceneRuntimeAssetNodeJointHintBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeJointHintNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeJointHintValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationState =
        rendererRuntime.sceneRuntimeAssetNodeArticulationState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeArticulationEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeArticulationResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationBrief =
        rendererRuntime.sceneRuntimeAssetNodeArticulationBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeArticulationNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeArticulationValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryJointBrief =
        rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryJointBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief =
        rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapState =
        rendererRuntime.sceneRuntimeAssetNodeArticulationMapState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeArticulationMapEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapBrief =
        rendererRuntime.sceneRuntimeAssetNodeArticulationMapBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeArticulationMapNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeArticulationMapValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintState =
        rendererRuntime.sceneRuntimeAssetNodeControlRigHintState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControlRigHintEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintBrief =
        rendererRuntime.sceneRuntimeAssetNodeControlRigHintBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeControlRigHintNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeControlRigHintValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelState =
        rendererRuntime.sceneRuntimeAssetNodeRigChannelState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeRigChannelEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeRigChannelResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelBrief =
        rendererRuntime.sceneRuntimeAssetNodeRigChannelBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeRigChannelNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeRigChannelValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceState =
        rendererRuntime.sceneRuntimeAssetNodeControlSurfaceState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControlSurfaceEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceBrief =
        rendererRuntime.sceneRuntimeAssetNodeControlSurfaceBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeControlSurfaceNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeControlSurfaceValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverState =
        rendererRuntime.sceneRuntimeAssetNodeRigDriverState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeRigDriverEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeRigDriverResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverBrief =
        rendererRuntime.sceneRuntimeAssetNodeRigDriverBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeRigDriverNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeRigDriverValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusState =
        rendererRuntime.sceneRuntimeAssetNodePoseBusState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseBusEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodePoseBusResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseBusBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusNameBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseBusNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusValueBrief =
        rendererRuntime.sceneRuntimeAssetNodePoseBusValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableState =
        rendererRuntime.sceneRuntimeAssetNodeControllerTableState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerTableEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerTableResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerTableBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerTableNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerTableValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeControllerRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusState =
        rendererRuntime.sceneRuntimeAssetNodeDriverBusState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeDriverBusEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeDriverBusResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusBrief =
        rendererRuntime.sceneRuntimeAssetNodeDriverBusBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeDriverBusNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeDriverBusValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionLaneState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionLaneEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionLaneBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionLaneNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionLaneValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseState =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief;
    diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief =
        rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief;
    diagnostics.rendererRuntime.sceneRuntimePoseAdapterInfluence =
        rendererRuntime.sceneRuntimePoseAdapterInfluence;
    diagnostics.rendererRuntime.sceneRuntimePoseReadabilityBias =
        rendererRuntime.sceneRuntimePoseReadabilityBias;
    diagnostics.rendererRuntime.sceneRuntimePoseAdapterBrief =
        rendererRuntime.sceneRuntimePoseAdapterBrief;
    diagnostics.rendererRuntime.facingDirection = rendererRuntime.facingDirection;
    diagnostics.rendererRuntime.surfaceWidth = rendererRuntime.surfaceWidth;
    diagnostics.rendererRuntime.surfaceHeight = rendererRuntime.surfaceHeight;
    diagnostics.rendererRuntime.modelSourceFormat = rendererRuntime.modelSourceFormat;
    diagnostics.rendererRuntime.appearanceSkinVariantId =
        rendererRuntime.appearanceSkinVariantId;
    diagnostics.rendererRuntime.appearanceAccessoryIds =
        rendererRuntime.appearanceAccessoryIds;
    diagnostics.rendererRuntime.appearanceAccessoryFamily =
        rendererRuntime.appearanceAccessoryFamily;
    diagnostics.rendererRuntime.appearanceComboPreset =
        rendererRuntime.appearanceComboPreset;
    diagnostics.rendererRuntime.appearanceRequestedPresetId =
        rendererRuntime.appearanceRequestedPresetId;
    diagnostics.rendererRuntime.appearanceResolvedPresetId =
        rendererRuntime.appearanceResolvedPresetId;
    diagnostics.rendererRuntime.appearancePluginId =
        rendererRuntime.appearancePluginId;
    diagnostics.rendererRuntime.appearancePluginKind =
        rendererRuntime.appearancePluginKind;
    diagnostics.rendererRuntime.appearancePluginSource =
        rendererRuntime.appearancePluginSource;
    diagnostics.rendererRuntime.appearancePluginSelectionReason =
        rendererRuntime.appearancePluginSelectionReason;
    diagnostics.rendererRuntime.appearancePluginFailureReason =
        rendererRuntime.appearancePluginFailureReason;
    diagnostics.rendererRuntime.appearancePluginManifestPath =
        rendererRuntime.appearancePluginManifestPath;
    diagnostics.rendererRuntime.appearancePluginRuntimeBackend =
        rendererRuntime.appearancePluginRuntimeBackend;
    diagnostics.rendererRuntime.appearancePluginMetadataPath =
        rendererRuntime.appearancePluginMetadataPath;
    diagnostics.rendererRuntime.appearancePluginMetadataSchemaVersion =
        rendererRuntime.appearancePluginMetadataSchemaVersion;
    diagnostics.rendererRuntime.appearancePluginAppearanceSemanticsMode =
        rendererRuntime.appearancePluginAppearanceSemanticsMode;
    diagnostics.rendererRuntime.appearancePluginSampleTier =
        rendererRuntime.appearancePluginSampleTier;
    diagnostics.rendererRuntime.appearancePluginContractBrief =
        rendererRuntime.appearancePluginContractBrief;
    diagnostics.rendererRuntime.defaultLaneCandidate =
        rendererRuntime.defaultLaneCandidate;
    diagnostics.rendererRuntime.defaultLaneSource =
        rendererRuntime.defaultLaneSource;
    diagnostics.rendererRuntime.defaultLaneRolloutStatus =
        rendererRuntime.defaultLaneRolloutStatus;
    diagnostics.rendererRuntime.defaultLaneStyleIntent =
        rendererRuntime.defaultLaneStyleIntent;
    diagnostics.rendererRuntime.defaultLaneCandidateTier =
        rendererRuntime.defaultLaneCandidateTier;
    for (const auto& descriptor : Win32MouseCompanionRendererBackendRegistry::Instance().ListByPriority()) {
        PetVisualHostRendererBackendCatalogEntry entry{};
        entry.name = descriptor.name;
        entry.priority = descriptor.priority;
        entry.available = descriptor.available;
        entry.unavailableReason = descriptor.unavailableReason;
        entry.unmetRequirements = descriptor.unmetRequirements;
        diagnostics.rendererBackendCatalog.push_back(std::move(entry));
    }
    return diagnostics;
}

void Win32MouseCompanionVisualHost::SyncWindow() {
    if (!state_.active || !state_.visible) {
        return;
    }
    window_.Update(state_);
}

void Win32MouseCompanionVisualHost::RefreshActionRuntime() {
    RefreshWin32MouseCompanionActionRuntimeSample(&state_, actionLibrary_, actionRuntime_, NowMs());
}

void Win32MouseCompanionVisualHost::UpdateActionRuntimeSelection(bool restartOneShot) {
    UpdateWin32MouseCompanionActionRuntimeSelection(state_, restartOneShot, NowMs(), &actionRuntime_);
}

} // namespace mousefx::windows
