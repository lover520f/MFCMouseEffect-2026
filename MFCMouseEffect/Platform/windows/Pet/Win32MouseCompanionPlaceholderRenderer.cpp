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
    diagnostics.sceneRuntimeModelNodeAdapterInfluence = 0.0f;
    diagnostics.sceneRuntimeModelNodeAdapterBrief =
        modelSceneAdapterProfile.seamState + "/0.00";
    diagnostics.sceneRuntimeModelNodeChannelBrief =
        "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
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
