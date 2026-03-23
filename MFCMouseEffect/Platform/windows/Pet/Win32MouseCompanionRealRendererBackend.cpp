#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererBackend.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererCapabilities.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPainter.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginHost.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendRegistry.h"

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

bool Win32MouseCompanionRealRendererBackend::Start() {
    lastErrorReason_ = DescribeWin32MouseCompanionRealRendererStartFailure();
    ready_ = lastErrorReason_.empty();
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_ = Win32MouseCompanionRendererBackendRuntimeDiagnostics{};
    runtimeDiagnostics_.backendName = "real";
    runtimeDiagnostics_.ready = ready_;
    return ready_;
}

void Win32MouseCompanionRealRendererBackend::Shutdown() {
    ready_ = false;
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_.ready = false;
    runtimeDiagnostics_.renderedFrame = false;
}

bool Win32MouseCompanionRealRendererBackend::IsReady() const {
    return ready_;
}

std::string Win32MouseCompanionRealRendererBackend::LastErrorReason() const {
    return lastErrorReason_;
}

void Win32MouseCompanionRealRendererBackend::Render(
    const Win32MouseCompanionRendererInput& input,
    Gdiplus::Graphics* graphics,
    int width,
    int height) const {
    if (!ready_ || !graphics || width <= 0 || height <= 0) {
        return;
    }
    graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    const auto resources = BuildWin32MouseCompanionRealRendererAssetResources(input);
    const auto sceneRuntime = BuildWin32MouseCompanionRealRendererSceneRuntime(input, resources);
    const auto scene = BuildWin32MouseCompanionRealRendererScene(sceneRuntime, width, height);
    const auto pluginSelection = ResolveWin32MouseCompanionRenderPluginSelection();
    const Win32MouseCompanionRealRendererPainter painter{};
    painter.Paint(scene, graphics, width, height);

    Win32MouseCompanionRendererBackendRuntimeDiagnostics diagnostics{};
    diagnostics.backendName = "real";
    diagnostics.ready = ready_;
    diagnostics.renderedFrame = true;
    diagnostics.renderedFrameCount = 1;
    diagnostics.lastRenderTickMs = ReadRendererRuntimeTickMs();
    diagnostics.actionName = sceneRuntime.actionName;
    diagnostics.reactiveActionName = sceneRuntime.reactiveActionName;
    diagnostics.actionIntensity = sceneRuntime.actionIntensity;
    diagnostics.reactiveActionIntensity = sceneRuntime.reactiveActionIntensity;
    diagnostics.modelReady = resources.modelReady;
    diagnostics.actionLibraryReady = resources.actionLibraryReady;
    diagnostics.appearanceProfileReady = resources.appearanceProfileReady;
    diagnostics.poseFrameAvailable = sceneRuntime.poseFrameAvailable;
    diagnostics.poseBindingConfigured = sceneRuntime.poseBindingConfigured;
    diagnostics.facingDirection = sceneRuntime.facingSign >= 0.0f ? 1 : -1;
    diagnostics.surfaceWidth = width;
    diagnostics.surfaceHeight = height;
    diagnostics.modelSourceFormat = resources.modelSourceFormat;
    diagnostics.appearanceSkinVariantId = resources.appearanceProfileSkinVariantId;
    diagnostics.appearanceAccessoryIds = resources.appearanceAccessoryIds;
    const auto accessoryFamily =
        ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
            resources.appearanceAccessoryIds);
    diagnostics.appearanceAccessoryFamily =
        ToStringWin32MouseCompanionRealRendererAppearanceAccessoryFamily(accessoryFamily);
    diagnostics.appearanceComboPreset =
        ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
            ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
                resources.appearanceProfileSkinVariantId,
                accessoryFamily));
    if (pluginSelection.comboPresetOverride !=
        Win32MouseCompanionRealRendererAppearanceComboPreset::None) {
        diagnostics.appearanceComboPreset =
            ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
                pluginSelection.comboPresetOverride);
    }
    diagnostics.appearanceRequestedPresetId = resources.appearanceRequestedPresetId;
    diagnostics.appearanceResolvedPresetId = resources.appearanceResolvedPresetId;
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
    diagnostics.defaultLaneCandidate = pluginSelection.defaultLaneCandidate;
    diagnostics.defaultLaneSource = pluginSelection.defaultLaneSource;
    diagnostics.defaultLaneRolloutStatus = pluginSelection.defaultLaneRolloutStatus;
    diagnostics.defaultLaneStyleIntent = pluginSelection.defaultLaneStyleIntent;
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    diagnostics.renderedFrameCount = runtimeDiagnostics_.renderedFrameCount + 1;
    runtimeDiagnostics_ = std::move(diagnostics);
}

Win32MouseCompanionRendererBackendRuntimeDiagnostics
Win32MouseCompanionRealRendererBackend::ReadRuntimeDiagnostics() const {
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    return runtimeDiagnostics_;
}

void RegisterWin32MouseCompanionRealRendererBackend() {
    static Win32MouseCompanionRendererBackendRegistrar<Win32MouseCompanionRealRendererBackend> registrar(
        "real",
        200,
        &EvaluateWin32MouseCompanionRealRendererAvailability);
    (void)registrar;
}

} // namespace mousefx::windows
