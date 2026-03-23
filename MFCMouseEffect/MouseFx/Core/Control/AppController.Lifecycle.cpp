#include "pch.h"

#include "AppController.h"

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Control/PetVisualAssetCoordinator.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Core/System/GdiPlusSession.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformPetVisualHost.h"
#include "Platform/PlatformRuntimeEnvironment.h"
#include "Platform/PlatformTarget.h"

#include <filesystem>
#include <vector>

namespace mousefx {
namespace {

constexpr uint32_t kPlatformInvalidHandleError = 6;

std::filesystem::path ResolveBundleResourcePath(const std::filesystem::path& relativePath) {
    const std::wstring exeDir = platform::GetExecutableDirectoryW();
    if (exeDir.empty()) {
        return {};
    }
    const std::filesystem::path macosDir(exeDir);
    const std::filesystem::path candidate = macosDir.parent_path() / L"Resources" / relativePath;
    std::error_code ec;
    if (candidate.empty() || !std::filesystem::exists(candidate, ec) || ec) {
        return {};
    }
    return candidate;
}

std::filesystem::path ResolveExistingDefaultPetModelPath(const std::string& configuredPathUtf8) {
    const std::string configuredPath = TrimAscii(configuredPathUtf8);
    if (!configuredPath.empty()) {
        const std::filesystem::path preferred(configuredPath);
        if (std::filesystem::exists(preferred)) {
            return preferred;
        }
    }

    static constexpr const char* kCandidates[] = {
        "Assets/Pet3D/source/pet-main.glb",
        "Assets/Pet3D/source/pet-main.vrm",
        "Assets/Pet3D/source/pet-main.usdz",
        "Assets/Pet3D/source/pet-main.gltf",
        "Assets/Pet3D/source/pet-main.fbx",
        "MFCMouseEffect/Assets/Pet3D/source/pet-main.glb",
        "MFCMouseEffect/Assets/Pet3D/source/pet-main.vrm",
        "MFCMouseEffect/Assets/Pet3D/source/pet-main.usdz",
        "MFCMouseEffect/Assets/Pet3D/source/pet-main.gltf",
        "MFCMouseEffect/Assets/Pet3D/source/pet-main.fbx",
    };

    for (const char* candidate : kCandidates) {
        const std::filesystem::path path(candidate);
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return ResolveBundleResourcePath(
        std::filesystem::path(L"MFCMouseEffect") / L"Assets" / L"Pet3D" / L"source" / L"pet-main.usdz");
}

std::filesystem::path ResolveExistingDefaultPetActionLibraryPath() {
    static constexpr const char* kCandidates[] = {
        "Assets/Pet3D/source/pet-actions.json",
        "MFCMouseEffect/Assets/Pet3D/source/pet-actions.json",
    };

    for (const char* candidate : kCandidates) {
        const std::filesystem::path path(candidate);
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return ResolveBundleResourcePath(
        std::filesystem::path(L"MFCMouseEffect") / L"Assets" / L"Pet3D" / L"source" / L"pet-actions.json");
}

std::filesystem::path ResolveExistingDefaultPetActionLibraryPath(const std::string& configuredPathUtf8) {
    const std::string configuredPath = TrimAscii(configuredPathUtf8);
    if (!configuredPath.empty()) {
        const std::filesystem::path preferred(configuredPath);
        if (std::filesystem::exists(preferred)) {
            return preferred;
        }
    }
    return ResolveExistingDefaultPetActionLibraryPath();
}

std::filesystem::path ResolveExistingDefaultPetAppearanceProfilePath() {
    static constexpr const char* kCandidates[] = {
        "Assets/Pet3D/source/pet-appearance.json",
        "MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json",
    };

    for (const char* candidate : kCandidates) {
        const std::filesystem::path path(candidate);
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return ResolveBundleResourcePath(
        std::filesystem::path(L"MFCMouseEffect") / L"Assets" / L"Pet3D" / L"source" / L"pet-appearance.json");
}

std::filesystem::path ResolveExistingDefaultPetAppearanceProfilePath(const std::string& configuredPathUtf8) {
    const std::string configuredPath = TrimAscii(configuredPathUtf8);
    if (!configuredPath.empty()) {
        const std::filesystem::path preferred(configuredPath);
        if (std::filesystem::exists(preferred)) {
            return preferred;
        }
    }
    return ResolveExistingDefaultPetAppearanceProfilePath();
}

std::string ResolveConfiguredPetEffectProfilePath() {
    static constexpr const char* kDefaultPath = "MFCMouseEffect/Assets/Pet3D/source/pet-effects.json";
    return kDefaultPath;
}

std::filesystem::path ResolveExistingDefaultPetEffectProfilePath() {
    static constexpr const char* kCandidates[] = {
        "Assets/Pet3D/source/pet-effects.json",
        "MFCMouseEffect/Assets/Pet3D/source/pet-effects.json",
    };
    for (const char* candidate : kCandidates) {
        const std::filesystem::path path(candidate);
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return ResolveBundleResourcePath(
        std::filesystem::path(L"MFCMouseEffect") / L"Assets" / L"Pet3D" / L"source" / L"pet-effects.json");
}

void AppendUniqueExistingPath(
    const std::filesystem::path& candidate,
    std::vector<std::filesystem::path>* outPaths) {
    if (!outPaths || candidate.empty()) {
        return;
    }
    std::error_code ec;
    if (!std::filesystem::exists(candidate, ec) || ec) {
        return;
    }
    for (const auto& existing : *outPaths) {
        if (existing == candidate) {
            return;
        }
    }
    outPaths->push_back(candidate);
}

std::vector<std::filesystem::path> BuildPetVisualModelCandidatePaths(
    const std::filesystem::path& preferredSourcePath,
    const std::string& canonicalGlbPath) {
    std::vector<std::filesystem::path> out;
    out.reserve(3);
    AppendUniqueExistingPath(preferredSourcePath, &out);

    const std::filesystem::path extProbe = preferredSourcePath.empty()
        ? std::filesystem::path(canonicalGlbPath)
        : preferredSourcePath;
    const std::string ext = TrimAscii(extProbe.extension().string());
    if (!ext.empty() && ext != ".usdz" && !extProbe.parent_path().empty() && !extProbe.stem().empty()) {
        AppendUniqueExistingPath(extProbe.parent_path() / (extProbe.stem().string() + ".usdz"), &out);
    }

    AppendUniqueExistingPath(std::filesystem::path(canonicalGlbPath), &out);
    return out;
}

void ResetActionCoverageStatusFields(AppController::MouseCompanionRuntimeStatus* status,
                                     const std::string& error = {}) {
    if (!status) {
        return;
    }
    status->actionCoverageReady = false;
    status->actionCoverageExpectedActionCount = 0;
    status->actionCoverageCoveredActionCount = 0;
    status->actionCoverageMissingActionCount = 0;
    status->actionCoverageSkeletonBoneCount = 0;
    status->actionCoverageTotalTrackCount = 0;
    status->actionCoverageMappedTrackCount = 0;
    status->actionCoverageOverallRatio = 0.0f;
    status->actionCoverageError = error;
    status->actionCoverageMissingActions.clear();
    status->actionCoverageMissingBoneNames.clear();
    status->actionCoverageActions.clear();
}

MouseCompanionPetRuntimeConfig BuildPetVisualHostConfig(const MouseCompanionConfig& config) {
    MouseCompanionPetRuntimeConfig runtime{};
    runtime.enabled = config.enabled;
    runtime.useTestProfile = config.useTestProfile;
    runtime.sizePx = config.sizePx;
    runtime.offsetX = config.offsetX;
    runtime.offsetY = config.offsetY;
    runtime.absoluteX = config.absoluteX;
    runtime.absoluteY = config.absoluteY;
    runtime.pressLiftPx = config.pressLiftPx;
    runtime.smoothingPercent = config.smoothingPercent;
    runtime.followThresholdPx = config.followThresholdPx;
    runtime.releaseHoldMs = config.releaseHoldMs;
    runtime.clickStreakBreakMs = config.clickStreakBreakMs;
    runtime.headTintPerClick = static_cast<float>(config.headTintPerClick);
    runtime.headTintMax = static_cast<float>(config.headTintMax);
    runtime.headTintDecayPerSecond = static_cast<float>(config.headTintDecayPerSecond);
    runtime.positionMode = config.positionMode;
    runtime.targetMonitor = config.targetMonitor;
    runtime.edgeClampMode = config.edgeClampMode;
    runtime.rendererBackendPreferenceSource = config.rendererBackendPreferenceSource;
    runtime.rendererBackendPreferenceName = config.rendererBackendPreferenceName;
    return runtime;
}

} // namespace

bool AppController::Start() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) return true;
    diag_ = {};
    inputCaptureActive_.store(false, std::memory_order_release);
    inputCaptureError_.store(0, std::memory_order_release);
    effectsSuspendedByInputCapture_.store(false, std::memory_order_release);
    ResetPetDispatchRuntimeState();

    // Load config from the best available directory (AppData preferred)
    configDir_ = ResolveConfigDirectory();
    config_ = EffectConfig::Load(configDir_);
    ResetMouseCompanionPluginHosts(config_.mouseCompanion, CurrentTickMs());
    {
        const MouseCompanionConfig companion = config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_ = MouseCompanionRuntimeStatus{};
        mouseCompanionRuntimeStatus_.configEnabled = companion.enabled;
        mouseCompanionRuntimeStatus_.configuredModelPath = companion.modelPath;
        mouseCompanionRuntimeStatus_.configuredActionLibraryPath = companion.actionLibraryPath;
        mouseCompanionRuntimeStatus_.configuredEffectProfilePath = ResolveConfiguredPetEffectProfilePath();
        mouseCompanionRuntimeStatus_.configuredAppearanceProfilePath = companion.appearanceProfilePath;
        mouseCompanionRuntimeStatus_.configuredRendererBackendPreferenceSource =
            companion.rendererBackendPreferenceSource;
        mouseCompanionRuntimeStatus_.configuredRendererBackendPreferenceName =
            companion.rendererBackendPreferenceName;
        mouseCompanionRuntimeStatus_.runtimePresent = false;
        mouseCompanionRuntimeStatus_.rendererRuntimeActionName = "idle";
        mouseCompanionRuntimeStatus_.rendererRuntimeReactiveActionName = "idle";
    }
    SyncLaunchAtStartupManifest();
    ReloadThemeCatalogFromRootPath(config_.themeCatalogRootPath);
    const bool themeNormalized = NormalizeConfiguredThemeName();
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(config_.holdPresenterBackend);
    ApplyOverlayTargetFpsToPlatform();
    InitializeWasmHost();
    inputIndicatorOverlay_->Initialize();
    inputIndicatorOverlay_->UpdateConfig(config_.inputIndicator);
    inputAutomationEngine_.UpdateConfig(config_.automation);
    if (config_.mouseCompanion.enabled) {
        TryLoadDefaultPetModel();
    }

    diag_.stage = StartStage::GdiPlusStartup;
    if (!gdiplus_ || !gdiplus_->Startup()) {
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: GDI+ startup failed.\n");
#endif
        return false;
    }

    diag_.stage = StartStage::DispatchWindow;
    if (!CreateDispatchWindow()) {
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: dispatch window creation failed.\n");
#endif
        Stop();
        return false;
    }

    // Initialize effects with defaults
    diag_.stage = StartStage::EffectInit;
    ApplyConfiguredEffects();
    inputIndicatorOverlay_->UpdateConfig(config_.inputIndicator);

    if (NormalizeActiveEffectTypes() || themeNormalized) {
        PersistConfig();
    }

    lastInputTime_ = CurrentTickMs();
    dispatchMessageHost_->SetTimer(kHoverTimerId, 100);
    dispatchMessageHost_->SetTimer(kInputCaptureHealthTimerId, 500);
    ArmWasmFrameTimer();

    diag_.stage = StartStage::GlobalHook;
    if (!hook_->Start(dispatchMessageHost_.get())) {
        const uint32_t hookError = hook_->LastError();
        inputCaptureActive_.store(false, std::memory_order_release);
        inputCaptureError_.store(hookError, std::memory_order_release);
#ifdef _DEBUG
        wchar_t buf[256]{};
        wsprintfW(buf, L"MouseFx: global hook start failed. GetLastError=%lu\n",
                  static_cast<unsigned long>(hookError));
        OutputDebugStringW(buf);
#endif
#if MFX_PLATFORM_WINDOWS
        diag_.error = static_cast<uint32_t>(hookError);
        Stop();
        return false;
#else
        // macOS/Linux: keep process alive in degraded mode when global capture is unavailable
        // (for example permission not granted yet). UI/tray and local services should continue.
        diag_.error = static_cast<uint32_t>(hookError);
        EnterInputCaptureDegradedMode(hookError);
#endif
    } else {
        inputCaptureActive_.store(true, std::memory_order_release);
        inputCaptureError_.store(0, std::memory_order_release);
        effectsSuspendedByInputCapture_.store(false, std::memory_order_release);
    }

    return true;
}

void AppController::Stop() {
    inputCaptureActive_.store(false, std::memory_order_release);
    inputCaptureError_.store(0, std::memory_order_release);
    effectsSuspendedByInputCapture_.store(false, std::memory_order_release);
    ResetMouseCompanionPluginHosts(config_.mouseCompanion, CurrentTickMs());
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoverTimerId);
        dispatchMessageHost_->KillTimer(kHoldTimerId);
        dispatchMessageHost_->KillTimer(kHoldUpdateTimerId);
        dispatchMessageHost_->KillTimer(kInputCaptureHealthTimerId);
        dispatchMessageHost_->KillTimer(kWasmFrameTimerId);
    }
    ShutdownWasmHost();
    hook_->SetKeyboardCaptureExclusive(false);
    hook_->Stop();
    inputIndicatorOverlay_->Shutdown();
    inputAutomationEngine_.Reset();
    ResetPetDispatchRuntimeState();
    ShutdownPetVisualHost();
    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.runtimePresent = false;
        mouseCompanionRuntimeStatus_.visualHostActive = false;
        mouseCompanionRuntimeStatus_.poseFrameAvailable = false;
        mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
    }
    ClearPetVisualHostDiagnostics();
    for (auto& effect : effects_) {
        if (effect) {
            effect->Shutdown();
            effect.reset();
        }
    }
    OverlayHostService::Instance().Shutdown();
    DestroyDispatchWindow();
    if (gdiplus_) {
        gdiplus_->Shutdown();
    }
}

bool AppController::CreateDispatchWindow() {
    if (!dispatchMessageHost_) {
        diag_.error = kPlatformInvalidHandleError;
        return false;
    }
    if (dispatchMessageHost_->IsCreated()) {
        return true;
    }
    if (!dispatchMessageHost_->Create(this)) {
        diag_.error = static_cast<uint32_t>(dispatchMessageHost_->LastError());
        return false;
    }
    return true;
}

void AppController::DestroyDispatchWindow() {
    if (dispatchMessageHost_) {
        dispatchMessageHost_->Destroy();
    }
}

void AppController::TryLoadDefaultPetModel() {
    loadedPetModelPath_.clear();
    loadedPetActionLibraryPath_.clear();
    loadedPetEffectProfilePath_.clear();
    loadedPetAppearanceProfilePath_.clear();
    petVisualPoseBindingConfigured_ = false;
    petVisualPoseBindingAttempted_ = false;
    const MouseCompanionConfig companion = config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
    if (!companion.enabled) {
        ShutdownPetVisualHost();
        {
            std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
            mouseCompanionRuntimeStatus_.runtimePresent = false;
            mouseCompanionRuntimeStatus_.visualHostActive = false;
            mouseCompanionRuntimeStatus_.visualModelLoaded = false;
            mouseCompanionRuntimeStatus_.modelLoaded = false;
            mouseCompanionRuntimeStatus_.actionLibraryLoaded = false;
            mouseCompanionRuntimeStatus_.effectProfileLoaded = false;
            mouseCompanionRuntimeStatus_.appearanceProfileLoaded = false;
            mouseCompanionRuntimeStatus_.poseFrameAvailable = false;
            mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
            mouseCompanionRuntimeStatus_.skeletonBoneCount = 0;
            mouseCompanionRuntimeStatus_.visualModelPath.clear();
            mouseCompanionRuntimeStatus_.loadedModelPath.clear();
            mouseCompanionRuntimeStatus_.loadedModelSourceFormat = "phase1_placeholder";
            mouseCompanionRuntimeStatus_.loadedActionLibraryPath.clear();
            mouseCompanionRuntimeStatus_.loadedEffectProfilePath.clear();
            mouseCompanionRuntimeStatus_.loadedAppearanceProfilePath.clear();
            mouseCompanionRuntimeStatus_.modelConvertedToCanonical = false;
            mouseCompanionRuntimeStatus_.modelImportDiagnostics.clear();
            mouseCompanionRuntimeStatus_.visualModelLoadError = "mouse_companion_disabled";
            mouseCompanionRuntimeStatus_.modelLoadError = "phase1_model_loader_pending";
            mouseCompanionRuntimeStatus_.actionLibraryLoadError = "phase1_action_library_pending";
            mouseCompanionRuntimeStatus_.effectProfileLoadError = "phase1_effect_profile_pending";
            mouseCompanionRuntimeStatus_.appearanceProfileLoadError = "phase1_appearance_profile_pending";
            mouseCompanionRuntimeStatus_.actionCoverageReady = false;
            mouseCompanionRuntimeStatus_.actionCoverageExpectedActionCount = 0;
            mouseCompanionRuntimeStatus_.actionCoverageCoveredActionCount = 0;
            mouseCompanionRuntimeStatus_.actionCoverageMissingActionCount = 0;
            mouseCompanionRuntimeStatus_.actionCoverageSkeletonBoneCount = 0;
            mouseCompanionRuntimeStatus_.actionCoverageTotalTrackCount = 0;
            mouseCompanionRuntimeStatus_.actionCoverageMappedTrackCount = 0;
            mouseCompanionRuntimeStatus_.actionCoverageOverallRatio = 0.0f;
            mouseCompanionRuntimeStatus_.actionCoverageError = "phase1_placeholder_no_skeleton";
            mouseCompanionRuntimeStatus_.actionCoverageMissingActions.clear();
            mouseCompanionRuntimeStatus_.actionCoverageMissingBoneNames.clear();
            mouseCompanionRuntimeStatus_.actionCoverageActions.clear();
        }
        ClearPetVisualHostDiagnostics();
        SyncMouseCompanionPluginPhase0Status();
        return;
    }

    EnsurePetVisualHost();
    ApplyPetVisualFollowProfile();

    const bool visualHostActive = petVisualHost_ && petVisualHost_->IsActive();
    const PetVisualHostDiagnostics visualHostDiagnostics =
        petVisualHost_ ? petVisualHost_->ReadDiagnostics() : PetVisualHostDiagnostics{};
    TryApplyPetModelToVisualHost();
    TryApplyPetActionLibraryToVisualHost();
    TryApplyPetAppearanceToVisualHost();
    const bool loadedVisualModel = !loadedPetModelPath_.empty();
    const bool loadedActionLibrary = !loadedPetActionLibraryPath_.empty();
    const bool loadedAppearanceProfile = !loadedPetAppearanceProfilePath_.empty();
    const bool poseBindingReady = visualHostActive && EnsurePetVisualPoseBinding();
    if (visualHostActive) {
        PetVisualHostUpdate idleUpdate{};
        petVisualHost_->Update(idleUpdate);
    }

    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.runtimePresent = visualHostActive;
        mouseCompanionRuntimeStatus_.visualHostActive = visualHostActive;
        mouseCompanionRuntimeStatus_.visualModelLoaded = visualHostActive;
        mouseCompanionRuntimeStatus_.modelLoaded = loadedVisualModel;
        mouseCompanionRuntimeStatus_.actionLibraryLoaded = loadedActionLibrary;
        mouseCompanionRuntimeStatus_.effectProfileLoaded = false;
        mouseCompanionRuntimeStatus_.appearanceProfileLoaded = loadedAppearanceProfile;
        mouseCompanionRuntimeStatus_.poseFrameAvailable = false;
        mouseCompanionRuntimeStatus_.poseBindingConfigured = poseBindingReady;
        mouseCompanionRuntimeStatus_.skeletonBoneCount = poseBindingReady ? 6 : 0;
        mouseCompanionRuntimeStatus_.loadedActionLibraryPath =
            loadedActionLibrary ? loadedPetActionLibraryPath_ : "";
        mouseCompanionRuntimeStatus_.loadedEffectProfilePath.clear();
        mouseCompanionRuntimeStatus_.loadedAppearanceProfilePath =
            loadedAppearanceProfile ? loadedPetAppearanceProfilePath_ : "";
        mouseCompanionRuntimeStatus_.modelConvertedToCanonical = false;
        mouseCompanionRuntimeStatus_.modelImportDiagnostics.clear();
        mouseCompanionRuntimeStatus_.actionLibraryLoadError =
            loadedActionLibrary ? "" : "phase2_visual_action_library_unavailable";
        mouseCompanionRuntimeStatus_.effectProfileLoadError = "phase1_effect_profile_pending";
        mouseCompanionRuntimeStatus_.appearanceProfileLoadError =
            loadedAppearanceProfile
                ? ""
                : (visualHostActive ? "phase1_appearance_profile_unavailable"
                                    : "phase1_visual_host_unavailable");
        mouseCompanionRuntimeStatus_.actionCoverageReady = false;
        mouseCompanionRuntimeStatus_.actionCoverageExpectedActionCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageCoveredActionCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageMissingActionCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageSkeletonBoneCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageTotalTrackCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageMappedTrackCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageOverallRatio = 0.0f;
        mouseCompanionRuntimeStatus_.actionCoverageError =
            poseBindingReady
                ? (loadedVisualModel ? "phase2_semantic_pose_scene_model" : "phase2_semantic_pose_placeholder")
                : "phase1_placeholder_no_skeleton";
        mouseCompanionRuntimeStatus_.actionCoverageMissingActions.clear();
        mouseCompanionRuntimeStatus_.actionCoverageMissingBoneNames.clear();
        mouseCompanionRuntimeStatus_.actionCoverageActions.clear();
    }
    SyncPetVisualHostDiagnostics(visualHostDiagnostics);
    SyncMouseCompanionPluginPhase0Status();
}

void AppController::TryLoadDefaultPetActionLibrary() {
    TryApplyPetActionLibraryToVisualHost();
}

void AppController::TryLoadDefaultPetAppearanceProfile() {
    TryApplyPetAppearanceToVisualHost();
}

void AppController::TryLoadDefaultPetEffectProfile() {
    TryLoadDefaultPetModel();
}

void AppController::RecomputePetActionCoverageStatus() {
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.actionCoverageReady = false;
    mouseCompanionRuntimeStatus_.actionCoverageExpectedActionCount = 0;
    mouseCompanionRuntimeStatus_.actionCoverageCoveredActionCount = 0;
    mouseCompanionRuntimeStatus_.actionCoverageMissingActionCount = 0;
    mouseCompanionRuntimeStatus_.actionCoverageSkeletonBoneCount = 0;
    mouseCompanionRuntimeStatus_.actionCoverageTotalTrackCount = 0;
    mouseCompanionRuntimeStatus_.actionCoverageMappedTrackCount = 0;
    mouseCompanionRuntimeStatus_.actionCoverageOverallRatio = 0.0f;
    mouseCompanionRuntimeStatus_.actionCoverageError = "backend_removed_pending_rewrite";
    mouseCompanionRuntimeStatus_.actionCoverageMissingActions.clear();
    mouseCompanionRuntimeStatus_.actionCoverageMissingBoneNames.clear();
    mouseCompanionRuntimeStatus_.actionCoverageActions.clear();
}

void AppController::SyncPetVisualHostDiagnostics(const PetVisualHostDiagnostics& diagnostics) {
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.preferredRendererBackendSource = diagnostics.preferredRendererBackendSource;
    mouseCompanionRuntimeStatus_.preferredRendererBackend = diagnostics.preferredRendererBackend;
    mouseCompanionRuntimeStatus_.selectedRendererBackend = diagnostics.selectedRendererBackend;
    mouseCompanionRuntimeStatus_.rendererBackendSelectionReason =
        diagnostics.rendererBackendSelectionReason;
    mouseCompanionRuntimeStatus_.rendererBackendFailureReason =
        diagnostics.rendererBackendFailureReason;
    mouseCompanionRuntimeStatus_.availableRendererBackends = diagnostics.availableRendererBackends;
    mouseCompanionRuntimeStatus_.unavailableRendererBackends = diagnostics.unavailableRendererBackends;
    mouseCompanionRuntimeStatus_.rendererBackendCatalog = diagnostics.rendererBackendCatalog;
    mouseCompanionRuntimeStatus_.realRendererUnmetRequirements.clear();
    for (const auto& entry : diagnostics.rendererBackendCatalog) {
        if (entry.name == "real") {
            mouseCompanionRuntimeStatus_.realRendererUnmetRequirements = entry.unmetRequirements;
            break;
        }
    }
    mouseCompanionRuntimeStatus_.rendererRuntimeBackend = diagnostics.rendererRuntime.backendName;
    mouseCompanionRuntimeStatus_.rendererRuntimeReady = diagnostics.rendererRuntime.ready;
    mouseCompanionRuntimeStatus_.rendererRuntimeFrameRendered = diagnostics.rendererRuntime.renderedFrame;
    mouseCompanionRuntimeStatus_.rendererRuntimeFrameCount =
        diagnostics.rendererRuntime.renderedFrameCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeLastRenderTickMs =
        diagnostics.rendererRuntime.lastRenderTickMs;
    mouseCompanionRuntimeStatus_.rendererRuntimeActionName = diagnostics.rendererRuntime.actionName;
    mouseCompanionRuntimeStatus_.rendererRuntimeReactiveActionName =
        diagnostics.rendererRuntime.reactiveActionName;
    mouseCompanionRuntimeStatus_.rendererRuntimeActionIntensity =
        diagnostics.rendererRuntime.actionIntensity;
    mouseCompanionRuntimeStatus_.rendererRuntimeReactiveActionIntensity =
        diagnostics.rendererRuntime.reactiveActionIntensity;
    mouseCompanionRuntimeStatus_.rendererRuntimeModelReady = diagnostics.rendererRuntime.modelReady;
    mouseCompanionRuntimeStatus_.rendererRuntimeActionLibraryReady =
        diagnostics.rendererRuntime.actionLibraryReady;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceProfileReady =
        diagnostics.rendererRuntime.appearanceProfileReady;
    mouseCompanionRuntimeStatus_.rendererRuntimePoseFrameAvailable =
        diagnostics.rendererRuntime.poseFrameAvailable;
    mouseCompanionRuntimeStatus_.rendererRuntimePoseBindingConfigured =
        diagnostics.rendererRuntime.poseBindingConfigured;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAdapterMode =
        diagnostics.rendererRuntime.sceneRuntimeAdapterMode;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseSampleCount =
        diagnostics.rendererRuntime.sceneRuntimePoseSampleCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeBoundPoseSampleCount =
        diagnostics.rendererRuntime.sceneRuntimeBoundPoseSampleCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelSceneAdapterState =
        diagnostics.rendererRuntime.sceneRuntimeModelSceneAdapterState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelSceneSeamReadiness =
        diagnostics.rendererRuntime.sceneRuntimeModelSceneSeamReadiness;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelSceneAdapterBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelSceneAdapterBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeAdapterInfluence =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeAdapterInfluence;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeAdapterBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeAdapterBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeChannelBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeChannelBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphState =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphNodeCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphNodeCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphBoundNodeCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphBoundNodeCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeGraphBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingState =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingBoundEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingBoundEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingWeightBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeBindingWeightBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotState =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeReadySlotCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeReadySlotCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeSlotNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryAssetNodeBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryAssetNodeBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryWeightBrief =
        diagnostics.rendererRuntime.sceneRuntimeModelNodeRegistryWeightBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingPathBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingPathBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingWeightBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeBindingWeightBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTransformState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTransformEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTransformResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTransformBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTransformPathBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformPathBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTransformValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTransformValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeAnchorState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeAnchorEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeAnchorResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeAnchorBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeAnchorPointBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorPointBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeAnchorScaleBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeAnchorScaleBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeResolverState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeResolverEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeResolverResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeResolverBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeResolverParentBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverParentBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeResolverValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeResolverValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeParentSpaceState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeParentSpaceEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeParentSpaceResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeParentSpaceBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeParentSpaceParentBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceParentBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeParentSpaceValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeParentSpaceValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetKindBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetKindBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetResolverState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetResolverEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetResolverResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetResolverBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetResolverPathBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverPathBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeTargetResolverValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeTargetResolverValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeWorldSpacePathBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpacePathBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeWorldSpaceValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePosePathBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePosePathBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseResolverState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseResolverEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseResolverResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseResolverBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseResolverPathBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverPathBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseResolverValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseResolverValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseRegistryNodeBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryNodeBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseRegistryWeightBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseRegistryWeightBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseChannelState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseChannelEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseChannelResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseChannelBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseChannelNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseChannelWeightBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseChannelWeightBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseConstraintState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseConstraintEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseConstraintResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseConstraintBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseConstraintNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseConstraintValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseConstraintValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseSolveState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseSolveEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseSolveResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseSolveBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseSolvePathBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolvePathBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseSolveValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseSolveValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeJointHintState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeJointHintEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeJointHintResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeJointHintBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeJointHintNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeJointHintValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeJointHintValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryJointBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryJointBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryWeightBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationMapState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationMapEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationMapResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationMapBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationMapNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeArticulationMapValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeArticulationMapValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlRigHintState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlRigHintEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlRigHintResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlRigHintBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlRigHintNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlRigHintValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlRigHintValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigChannelState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigChannelEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigChannelResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigChannelBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigChannelNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigChannelValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigChannelValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControlSurfaceValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigDriverState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigDriverEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigDriverResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigDriverBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigDriverNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeRigDriverValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeRigDriverValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceDriverValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseBusState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseBusEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseBusResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseBusBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseBusNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodePoseBusValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodePoseBusValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerTableState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerTableEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerTableResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerTableBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerTableNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerTableValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerTableValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerRegistryValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeDriverBusState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeDriverBusEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeDriverBusResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeDriverBusBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeDriverBusNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeDriverBusValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeDriverBusValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerDriverRegistryValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionLaneValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionSurfaceValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeCompositionRegistryValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverTableValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableState =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableState;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief =
        diagnostics.rendererRuntime.sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseAdapterInfluence =
        diagnostics.rendererRuntime.sceneRuntimePoseAdapterInfluence;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseReadabilityBias =
        diagnostics.rendererRuntime.sceneRuntimePoseReadabilityBias;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseAdapterBrief =
        diagnostics.rendererRuntime.sceneRuntimePoseAdapterBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeFacingDirection =
        diagnostics.rendererRuntime.facingDirection;
    mouseCompanionRuntimeStatus_.rendererRuntimeSurfaceWidth =
        diagnostics.rendererRuntime.surfaceWidth;
    mouseCompanionRuntimeStatus_.rendererRuntimeSurfaceHeight =
        diagnostics.rendererRuntime.surfaceHeight;
    mouseCompanionRuntimeStatus_.rendererRuntimeModelSourceFormat =
        diagnostics.rendererRuntime.modelSourceFormat;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceSkinVariantId =
        diagnostics.rendererRuntime.appearanceSkinVariantId;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceAccessoryIds =
        diagnostics.rendererRuntime.appearanceAccessoryIds;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceAccessoryFamily =
        diagnostics.rendererRuntime.appearanceAccessoryFamily;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceComboPreset =
        diagnostics.rendererRuntime.appearanceComboPreset;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceRequestedPresetId =
        diagnostics.rendererRuntime.appearanceRequestedPresetId;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceResolvedPresetId =
        diagnostics.rendererRuntime.appearanceResolvedPresetId;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginId =
        diagnostics.rendererRuntime.appearancePluginId;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginKind =
        diagnostics.rendererRuntime.appearancePluginKind;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginSource =
        diagnostics.rendererRuntime.appearancePluginSource;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginSelectionReason =
        diagnostics.rendererRuntime.appearancePluginSelectionReason;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginFailureReason =
        diagnostics.rendererRuntime.appearancePluginFailureReason;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginManifestPath =
        diagnostics.rendererRuntime.appearancePluginManifestPath;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginRuntimeBackend =
        diagnostics.rendererRuntime.appearancePluginRuntimeBackend;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginMetadataPath =
        diagnostics.rendererRuntime.appearancePluginMetadataPath;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginMetadataSchemaVersion =
        diagnostics.rendererRuntime.appearancePluginMetadataSchemaVersion;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginAppearanceSemanticsMode =
        diagnostics.rendererRuntime.appearancePluginAppearanceSemanticsMode;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginSampleTier =
        diagnostics.rendererRuntime.appearancePluginSampleTier;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginContractBrief =
        diagnostics.rendererRuntime.appearancePluginContractBrief;
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneCandidate =
        diagnostics.rendererRuntime.defaultLaneCandidate;
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneSource =
        diagnostics.rendererRuntime.defaultLaneSource;
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneRolloutStatus =
        diagnostics.rendererRuntime.defaultLaneRolloutStatus;
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneStyleIntent =
        diagnostics.rendererRuntime.defaultLaneStyleIntent;
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneCandidateTier =
        diagnostics.rendererRuntime.defaultLaneCandidateTier;
}

void AppController::ClearPetVisualHostDiagnostics() {
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.preferredRendererBackendSource.clear();
    mouseCompanionRuntimeStatus_.preferredRendererBackend.clear();
    mouseCompanionRuntimeStatus_.selectedRendererBackend.clear();
    mouseCompanionRuntimeStatus_.rendererBackendSelectionReason.clear();
    mouseCompanionRuntimeStatus_.rendererBackendFailureReason.clear();
    mouseCompanionRuntimeStatus_.availableRendererBackends.clear();
    mouseCompanionRuntimeStatus_.unavailableRendererBackends.clear();
    mouseCompanionRuntimeStatus_.rendererBackendCatalog.clear();
    mouseCompanionRuntimeStatus_.realRendererUnmetRequirements.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeBackend.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeReady = false;
    mouseCompanionRuntimeStatus_.rendererRuntimeFrameRendered = false;
    mouseCompanionRuntimeStatus_.rendererRuntimeFrameCount = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeLastRenderTickMs = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeActionName = "idle";
    mouseCompanionRuntimeStatus_.rendererRuntimeReactiveActionName = "idle";
    mouseCompanionRuntimeStatus_.rendererRuntimeActionIntensity = 0.0f;
    mouseCompanionRuntimeStatus_.rendererRuntimeReactiveActionIntensity = 0.0f;
    mouseCompanionRuntimeStatus_.rendererRuntimeModelReady = false;
    mouseCompanionRuntimeStatus_.rendererRuntimeActionLibraryReady = false;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceProfileReady = false;
    mouseCompanionRuntimeStatus_.rendererRuntimePoseFrameAvailable = false;
    mouseCompanionRuntimeStatus_.rendererRuntimePoseBindingConfigured = false;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAdapterMode = "runtime_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseSampleCount = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeBoundPoseSampleCount = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelSceneAdapterState =
        "preview_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelSceneSeamReadiness =
        0.0f;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelSceneAdapterBrief =
        "preview_only/unknown/runtime_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeAdapterInfluence =
        0.0f;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeAdapterBrief =
        "preview_only/0.00";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeChannelBrief =
        "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphState =
        "preview_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphNodeCount =
        0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphBoundNodeCount =
        0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeGraphBrief =
        "preview_only/0/0";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingState =
        "preview_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingEntryCount =
        0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingBoundEntryCount =
        0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingBrief =
        "preview_only/0/0";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeBindingWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotState =
        "preview_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotCount =
        0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeReadySlotCount =
        0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotBrief =
        "preview_only/0/0";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeSlotNameBrief =
        "body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryState =
        "preview_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryEntryCount = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryResolvedEntryCount = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryBrief =
        "preview_only/0/0";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryAssetNodeBrief =
        "body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeModelNodeRegistryWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingState =
        "preview_only";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingEntryCount = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingResolvedEntryCount = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingBrief =
        "preview_only/0/0";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimeAssetNodeBindingWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseAdapterInfluence = 0.0f;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseReadabilityBias = 0.0f;
    mouseCompanionRuntimeStatus_.rendererRuntimeSceneRuntimePoseAdapterBrief =
        "runtime_only/0.00/0.00";
    mouseCompanionRuntimeStatus_.rendererRuntimeFacingDirection = 1;
    mouseCompanionRuntimeStatus_.rendererRuntimeSurfaceWidth = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeSurfaceHeight = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeModelSourceFormat = "unknown";
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceSkinVariantId = "default";
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceAccessoryIds.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceAccessoryFamily = "none";
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceComboPreset = "none";
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceRequestedPresetId.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearanceResolvedPresetId.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginId.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginKind.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginSource.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginSelectionReason.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginFailureReason.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginManifestPath.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginRuntimeBackend.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginMetadataPath.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginMetadataSchemaVersion = 0;
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginAppearanceSemanticsMode =
        "legacy_manifest_compat";
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginSampleTier.clear();
    mouseCompanionRuntimeStatus_.rendererRuntimeAppearancePluginContractBrief =
        "legacy_manifest_compat/-/-";
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneCandidate = "builtin";
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneSource =
        "runtime_builtin_default";
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneRolloutStatus =
        "stay_on_builtin";
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneStyleIntent =
        "style_candidate:none";
    mouseCompanionRuntimeStatus_.rendererRuntimeDefaultLaneCandidateTier =
        "builtin_shipped_default";
}

void AppController::EnsurePetVisualHost() {
    const MouseCompanionConfig companion = config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
    if (!petVisualHost_) {
        petVisualHost_ = platform::CreatePetVisualHost();
    }
    if (petVisualHost_ && !petVisualHost_->IsActive()) {
        petVisualHost_->Start(BuildPetVisualHostConfig(companion));
    }
    if (petVisualHost_ && petVisualHost_->IsActive()) {
        ApplyPetVisualFollowProfile();
        petVisualHost_->Show();
    }
    const bool visualHostActive = petVisualHost_ && petVisualHost_->IsActive();
    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.visualHostActive = visualHostActive;
    }
    if (visualHostActive) {
        SyncPetVisualHostDiagnostics(petVisualHost_->ReadDiagnostics());
    } else {
        ClearPetVisualHostDiagnostics();
    }
}

void AppController::ApplyPetVisualFollowProfile() {
    if (!petVisualHost_ || !petVisualHost_->IsActive()) {
        return;
    }
    const MouseCompanionConfig companion = config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
    petVisualHost_->Configure(BuildPetVisualHostConfig(companion));
}

void AppController::ShutdownPetVisualHost() {
    if (petVisualHost_) {
        petVisualHost_->Hide();
        petVisualHost_->Shutdown();
    }
    petVisualHost_.reset();
    petVisualPoseBindingConfigured_ = false;
    petVisualPoseBindingAttempted_ = false;
    petVisualPoseRuntime_ = PetVisualPoseRuntimeState{};
    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.visualHostActive = false;
        mouseCompanionRuntimeStatus_.visualModelLoaded = false;
        mouseCompanionRuntimeStatus_.poseFrameAvailable = false;
        mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
        mouseCompanionRuntimeStatus_.visualModelPath.clear();
        mouseCompanionRuntimeStatus_.visualModelLoadError = "phase1_visual_host_inactive";
    }
    ClearPetVisualHostDiagnostics();
}

bool AppController::TryLoadPetModelIntoVisualHost(const std::string& modelPath) {
    if (!petVisualHost_ || !petVisualHost_->IsActive()) {
        return false;
    }
    const std::string trimmed = TrimAscii(modelPath);
    if (trimmed.empty()) {
        return false;
    }
    return petVisualHost_->LoadModel(trimmed);
}

bool AppController::TryLoadPetActionLibraryIntoVisualHost(const std::string& actionLibraryPath) {
    if (!petVisualHost_ || !petVisualHost_->IsActive()) {
        return false;
    }
    const std::string trimmed = TrimAscii(actionLibraryPath);
    if (trimmed.empty()) {
        return false;
    }
    return petVisualHost_->LoadActionLibrary(trimmed);
}

bool AppController::TryLoadPetAppearanceProfileIntoVisualHost(const std::string& appearanceProfilePath) {
    if (!petVisualHost_ || !petVisualHost_->IsActive()) {
        return false;
    }
    const std::string trimmed = TrimAscii(appearanceProfilePath);
    if (trimmed.empty()) {
        return false;
    }
    return petVisualHost_->LoadAppearanceProfile(trimmed);
}

void AppController::TryApplyPetModelToVisualHost() {
    loadedPetModelPath_.clear();

    const MouseCompanionConfig companion =
        config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
    const bool visualHostActive = petVisualHost_ && petVisualHost_->IsActive();
    const std::filesystem::path preferredModelPath =
        ResolveExistingDefaultPetModelPath(companion.modelPath);
    const PetVisualAssetApplyResult result = ApplyPetVisualModelAsset(
        companion.enabled,
        visualHostActive,
        BuildPetVisualModelCandidatePaths(
            preferredModelPath,
            "MFCMouseEffect/Assets/Pet3D/source/pet-main.glb"),
        [this](const std::string& path) { return TryLoadPetModelIntoVisualHost(path); });
    loadedPetModelPath_ = result.loadedPath;

    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.modelLoaded = result.loaded;
    mouseCompanionRuntimeStatus_.loadedModelPath = result.loadedPath;
    mouseCompanionRuntimeStatus_.loadedModelSourceFormat =
        ResolvePetVisualModelSourceFormat(result.loadedPath);
    mouseCompanionRuntimeStatus_.visualModelPath =
        result.loaded ? result.loadedPath : (visualHostActive ? "phase1://placeholder/usagi" : "");
    mouseCompanionRuntimeStatus_.visualModelLoadError =
        visualHostActive ? "" : "phase1_visual_host_unavailable";
    mouseCompanionRuntimeStatus_.modelLoadError = result.loaded ? "" : result.loadError;
}

void AppController::TryApplyPetActionLibraryToVisualHost() {
    loadedPetActionLibraryPath_.clear();

    const MouseCompanionConfig companion =
        config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
    const bool visualHostActive = petVisualHost_ && petVisualHost_->IsActive();
    const PetVisualAssetApplyResult result = ApplyPetVisualSinglePathAsset(
        companion.enabled,
        visualHostActive,
        ResolveExistingDefaultPetActionLibraryPath(companion.actionLibraryPath),
        [this](const std::string& path) { return TryLoadPetActionLibraryIntoVisualHost(path); },
        "phase2_visual_action_library_missing",
        "phase2_visual_action_library_unavailable");
    loadedPetActionLibraryPath_ = result.loadedPath;

    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.actionLibraryLoaded = result.loaded;
    mouseCompanionRuntimeStatus_.loadedActionLibraryPath = result.loadedPath;
    mouseCompanionRuntimeStatus_.actionLibraryLoadError = result.loaded ? "" : result.loadError;
}

void AppController::TryApplyPetAppearanceToVisualHost() {
    loadedPetAppearanceProfilePath_.clear();

    const MouseCompanionConfig companion =
        config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
    const bool visualHostActive = petVisualHost_ && petVisualHost_->IsActive();
    const PetVisualAssetApplyResult result = ApplyPetVisualSinglePathAsset(
        companion.enabled,
        visualHostActive,
        ResolveExistingDefaultPetAppearanceProfilePath(companion.appearanceProfilePath),
        [this](const std::string& path) { return TryLoadPetAppearanceProfileIntoVisualHost(path); },
        "phase1_appearance_profile_missing",
        "phase1_appearance_profile_unavailable");
    loadedPetAppearanceProfilePath_ = result.loadedPath;

    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.appearanceProfileLoaded = result.loaded;
    mouseCompanionRuntimeStatus_.loadedAppearanceProfilePath = result.loadedPath;
    mouseCompanionRuntimeStatus_.appearanceProfileLoadError =
        result.loaded ? "" : result.loadError;
}

} // namespace mousefx
