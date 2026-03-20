#include "pch.h"

#include "AppController.h"

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Core/System/GdiPlusSession.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformRuntimeEnvironment.h"
#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Pet/MacosMouseCompanionPhase1SwiftBridge.h"
#endif

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

int ResolveMouseCompanionEdgeClampModeCode(const std::string& mode) {
    const std::string normalized = ToLowerAscii(TrimAscii(mode));
    if (normalized == "strict") {
        return 0;
    }
    if (normalized == "free") {
        return 2;
    }
    return 1; // soft
}

int ResolveMouseCompanionPositionModeCode(const std::string& mode) {
    const std::string normalized = ToLowerAscii(TrimAscii(mode));
    if (normalized == "relative" || normalized == "follow") {
        return 0;
    }
    if (normalized == "absolute") {
        return 1;
    }
    return 2; // fixed_bottom_left legacy compatibility
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
        mouseCompanionRuntimeStatus_.runtimePresent = false;
    }
    SyncLaunchAtStartupRegistration();
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
        mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
    }
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
        SyncMouseCompanionPluginPhase0Status();
        return;
    }

    EnsurePetVisualHost();
    ApplyPetVisualFollowProfile();

    const bool visualHostActive = (petVisualHostHandle_ != nullptr);
    bool loadedVisualModel = false;
    std::filesystem::path loadedVisualModelPath;
    bool loadedActionLibrary = false;
    std::filesystem::path loadedActionLibraryPath;
    if (visualHostActive) {
        const std::filesystem::path preferredModelPath =
            ResolveExistingDefaultPetModelPath(companion.modelPath);
        const std::vector<std::filesystem::path> modelCandidates =
            BuildPetVisualModelCandidatePaths(
                preferredModelPath,
                "MFCMouseEffect/Assets/Pet3D/source/pet-main.glb");
        for (const auto& candidate : modelCandidates) {
            if (candidate.empty()) {
                continue;
            }
            if (TryLoadPetModelIntoVisualHost(candidate.string())) {
                loadedVisualModel = true;
                loadedVisualModelPath = candidate;
                break;
            }
        }
        if (loadedVisualModel) {
            loadedPetModelPath_ = loadedVisualModelPath.string();
        }

        const std::filesystem::path preferredActionLibraryPath =
            ResolveExistingDefaultPetActionLibraryPath(companion.actionLibraryPath);
        if (!preferredActionLibraryPath.empty() &&
            TryLoadPetActionLibraryIntoVisualHost(preferredActionLibraryPath.string())) {
            loadedActionLibrary = true;
            loadedActionLibraryPath = preferredActionLibraryPath;
            loadedPetActionLibraryPath_ = loadedActionLibraryPath.string();
        }
    }
    const bool poseBindingReady = visualHostActive && EnsurePetVisualPoseBinding();
#if MFX_PLATFORM_MACOS
    if (visualHostActive) {
        mfx_macos_mouse_companion_panel_update_v1(
            petVisualHostHandle_,
            0,
            0.0f,
            0.0f);
    }
#endif

    std::string loadedModelSourceFormat = "phase1_placeholder";
    if (loadedVisualModel) {
        std::string ext = ToLowerAscii(TrimAscii(loadedVisualModelPath.extension().string()));
        if (!ext.empty() && ext[0] == '.') {
            ext.erase(0, 1);
        }
        if (!ext.empty()) {
            loadedModelSourceFormat = ext;
        }
    }

    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.runtimePresent = visualHostActive;
        mouseCompanionRuntimeStatus_.visualHostActive = visualHostActive;
        mouseCompanionRuntimeStatus_.visualModelLoaded = visualHostActive;
        mouseCompanionRuntimeStatus_.modelLoaded = loadedVisualModel;
        mouseCompanionRuntimeStatus_.actionLibraryLoaded = loadedActionLibrary;
        mouseCompanionRuntimeStatus_.effectProfileLoaded = false;
        mouseCompanionRuntimeStatus_.appearanceProfileLoaded = false;
        mouseCompanionRuntimeStatus_.poseBindingConfigured = poseBindingReady;
        mouseCompanionRuntimeStatus_.skeletonBoneCount = poseBindingReady ? 6 : 0;
        mouseCompanionRuntimeStatus_.visualModelPath =
            loadedVisualModel ? loadedPetModelPath_ : (visualHostActive ? "phase1://placeholder/usagi" : "");
        mouseCompanionRuntimeStatus_.loadedModelPath = loadedVisualModel ? loadedPetModelPath_ : "";
        mouseCompanionRuntimeStatus_.loadedModelSourceFormat = loadedModelSourceFormat;
        mouseCompanionRuntimeStatus_.loadedActionLibraryPath =
            loadedActionLibrary ? loadedPetActionLibraryPath_ : "";
        mouseCompanionRuntimeStatus_.loadedEffectProfilePath.clear();
        mouseCompanionRuntimeStatus_.loadedAppearanceProfilePath.clear();
        mouseCompanionRuntimeStatus_.modelConvertedToCanonical = false;
        mouseCompanionRuntimeStatus_.modelImportDiagnostics.clear();
        mouseCompanionRuntimeStatus_.visualModelLoadError =
            visualHostActive ? "" : "phase1_swift_host_unavailable";
        mouseCompanionRuntimeStatus_.modelLoadError =
            loadedVisualModel ? "" : "phase2_visual_model_fallback_placeholder";
        mouseCompanionRuntimeStatus_.actionLibraryLoadError =
            loadedActionLibrary ? "" : "phase2_visual_action_library_unavailable";
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
        mouseCompanionRuntimeStatus_.actionCoverageError =
            poseBindingReady
                ? (loadedVisualModel ? "phase2_semantic_pose_scene_model" : "phase2_semantic_pose_placeholder")
                : "phase1_placeholder_no_skeleton";
        mouseCompanionRuntimeStatus_.actionCoverageMissingActions.clear();
        mouseCompanionRuntimeStatus_.actionCoverageMissingBoneNames.clear();
        mouseCompanionRuntimeStatus_.actionCoverageActions.clear();
    }
    SyncMouseCompanionPluginPhase0Status();
}

void AppController::TryLoadDefaultPetActionLibrary() {
    TryLoadDefaultPetModel();
}

void AppController::TryLoadDefaultPetAppearanceProfile() {
    TryLoadDefaultPetModel();
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

void AppController::EnsurePetVisualHost() {
#if MFX_PLATFORM_MACOS
    if (!petVisualHostHandle_) {
        const MouseCompanionConfig companion = config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
        petVisualHostHandle_ = mfx_macos_mouse_companion_panel_create_v1(
            companion.sizePx,
            companion.offsetX,
            companion.offsetY);
    }
    if (petVisualHostHandle_) {
        ApplyPetVisualFollowProfile();
        mfx_macos_mouse_companion_panel_show_v1(petVisualHostHandle_);
    }
#endif
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.visualHostActive = (petVisualHostHandle_ != nullptr);
}

void AppController::ApplyPetVisualFollowProfile() {
#if MFX_PLATFORM_MACOS
    if (!petVisualHostHandle_) {
        return;
    }
    const MouseCompanionConfig companion = config_internal::SanitizeMouseCompanionConfig(config_.mouseCompanion);
    mfx_macos_mouse_companion_panel_configure_v1(
        petVisualHostHandle_,
        companion.sizePx,
        ResolveMouseCompanionPositionModeCode(companion.positionMode),
        ResolveMouseCompanionEdgeClampModeCode(companion.edgeClampMode),
        companion.offsetX,
        companion.offsetY,
        companion.absoluteX,
        companion.absoluteY,
        companion.targetMonitor.c_str());
#endif
}

void AppController::ShutdownPetVisualHost() {
#if MFX_PLATFORM_MACOS
    if (petVisualHostHandle_) {
        mfx_macos_mouse_companion_panel_hide_v1(petVisualHostHandle_);
        mfx_macos_mouse_companion_panel_release_v1(petVisualHostHandle_);
    }
#endif
    petVisualHostHandle_ = nullptr;
    petVisualPoseBindingConfigured_ = false;
    petVisualPoseBindingAttempted_ = false;
    petVisualPoseRuntime_ = PetVisualPoseRuntimeState{};
    petVisualSkeletonNamePtrs_.clear();
    petVisualSkeletonNames_.clear();
    std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
    mouseCompanionRuntimeStatus_.visualHostActive = false;
    mouseCompanionRuntimeStatus_.visualModelLoaded = false;
    mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
    mouseCompanionRuntimeStatus_.visualModelPath.clear();
    mouseCompanionRuntimeStatus_.visualModelLoadError = "phase1_visual_host_inactive";
}

bool AppController::TryLoadPetModelIntoVisualHost(const std::string& modelPath) {
#if MFX_PLATFORM_MACOS
    if (!petVisualHostHandle_) {
        return false;
    }
    const std::string trimmed = TrimAscii(modelPath);
    if (trimmed.empty()) {
        return false;
    }
    return mfx_macos_mouse_companion_panel_load_model_v1(
        petVisualHostHandle_,
        trimmed.c_str());
#else
    (void)modelPath;
    return false;
#endif
}

bool AppController::TryLoadPetActionLibraryIntoVisualHost(const std::string& actionLibraryPath) {
#if MFX_PLATFORM_MACOS
    if (!petVisualHostHandle_) {
        return false;
    }
    const std::string trimmed = TrimAscii(actionLibraryPath);
    if (trimmed.empty()) {
        return false;
    }
    return mfx_macos_mouse_companion_panel_load_action_library_v1(
        petVisualHostHandle_,
        trimmed.c_str());
#else
    (void)actionLibraryPath;
    return false;
#endif
}

} // namespace mousefx
