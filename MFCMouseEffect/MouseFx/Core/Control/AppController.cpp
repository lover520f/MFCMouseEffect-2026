// AppController.cpp

#include "pch.h"

#include "AppController.h"
#include "CommandHandler.h"
#include "DispatchRouter.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"
#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Control/EffectFactory.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Core/Control/NullDispatchMessageHost.h"
#include "MouseFx/Core/Control/NullDispatchMessageCodec.h"
#include "MouseFx/Core/System/NullCursorPositionService.h"
#include "MouseFx/Core/System/NullForegroundProcessService.h"
#include "MouseFx/Core/System/GdiPlusSession.h"
#include "MouseFx/Core/System/StdMonotonicClockService.h"
#include "MouseFx/Core/Overlay/NullInputIndicatorOverlay.h"
#include "MouseFx/Core/Protocol/JsonLite.h"
#include "MouseFx/Core/Wasm/WasmClickCommandExecutor.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Effects/HoldRouteCatalog.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"
#include "MouseFx/Core/System/VmForegroundDetector.h"
#include "Platform/PlatformControlServicesFactory.h"
#include "Platform/PlatformControlMessageCodecFactory.h"
#include "Platform/PlatformInputServicesFactory.h"
#include "Platform/PlatformOverlayServicesFactory.h"
#include "Platform/PlatformSystemServicesFactory.h"

#include <new>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace mousefx {

using json = nlohmann::json;

static std::string NormalizeHoldFollowMode(std::string mode) {
    mode = ToLowerAscii(mode);
    if (mode == "precise") return "precise";
    if (mode == "efficient") return "efficient";
    return "smooth";
}

struct ActiveCategoryDescriptor {
    EffectCategory category;
    std::string ActiveEffectConfig::*slot;
    bool themeSensitive = false;
};

constexpr std::array<ActiveCategoryDescriptor, 5> kActiveCategoryDescriptors{{
    {EffectCategory::Click, &ActiveEffectConfig::click, false},
    {EffectCategory::Trail, &ActiveEffectConfig::trail, false},
    {EffectCategory::Scroll, &ActiveEffectConfig::scroll, true},
    {EffectCategory::Hold, &ActiveEffectConfig::hold, true},
    {EffectCategory::Hover, &ActiveEffectConfig::hover, true},
}};


AppController::AppController()
    : gdiplus_(std::make_unique<GdiPlusSession>())
    , dispatchMessageHost_(platform::CreateDispatchMessageHost())
    , dispatchMessageCodec_(platform::CreateDispatchMessageCodec())
    , cursorPositionService_(platform::CreateCursorPositionService())
    , monotonicClockService_(platform::CreateMonotonicClockService())
    , foregroundProcessService_(platform::CreateForegroundProcessService())
    , hook_(platform::CreateGlobalMouseHook())
    , inputIndicatorOverlay_(platform::CreateInputIndicatorOverlay())
    , commandHandler_(std::make_unique<CommandHandler>(this))
    , dispatchRouter_(std::make_unique<DispatchRouter>(this))
    , vmForegroundDetector_(std::make_unique<VmForegroundDetector>()) {
    if (!dispatchMessageHost_) {
        dispatchMessageHost_ = std::make_unique<NullDispatchMessageHost>();
    }
    if (!dispatchMessageCodec_) {
        dispatchMessageCodec_ = std::make_unique<NullDispatchMessageCodec>();
    }
    if (!cursorPositionService_) {
        cursorPositionService_ = std::make_unique<NullCursorPositionService>();
    }
    if (!monotonicClockService_) {
        monotonicClockService_ = std::make_unique<StdMonotonicClockService>();
    }
    if (!foregroundProcessService_) {
        foregroundProcessService_ = std::make_unique<NullForegroundProcessService>();
    }
    shortcutCaptureSession_.SetClockService(monotonicClockService_.get());
    inputAutomationEngine_.SetForegroundProcessService(foregroundProcessService_.get());
    if (!inputIndicatorOverlay_) {
        inputIndicatorOverlay_ = std::make_unique<NullInputIndicatorOverlay>();
    }
}

AppController::~AppController() {
    Stop();
}

EffectConfig AppController::GetConfigSnapshot() const {
    if (!dispatchMessageHost_ || !dispatchMessageHost_->IsCreated()) {
        return config_;
    }
    if (dispatchMessageHost_->IsOwnerThread()) {
        return config_;
    }

    EffectConfig snapshot{};
    dispatchMessageHost_->SendSync(WM_MFX_GET_CONFIG, 0, reinterpret_cast<intptr_t>(&snapshot));
    return snapshot;
}

void AppController::PersistConfig() {
    if (!configDir_.empty()) {
        EffectConfig::Save(configDir_, config_);
    }
}

std::string AppController::ResolveRuntimeEffectType(
    EffectCategory category,
    const std::string& requestedType,
    std::string* outReason) const {
    if (outReason) outReason->clear();
    if (category != EffectCategory::Hold) {
        return requestedType;
    }
    const std::string normalizedType = hold_route::NormalizeHoldEffectTypeAlias(requestedType);
    const char* reason = hold_route::RouteReasonForType(normalizedType);
    if (outReason && reason && reason[0] != '\0') {
        *outReason = reason;
    }
    return normalizedType;
}

void AppController::NotifyGpuFallbackIfNeeded(const std::string& reason) {
    if (gpuFallbackNotifiedThisSession_) return;
    gpuFallbackNotifiedThisSession_ = true;
    // UX decision: do not block runtime with modal dialogs.
    // Fallback status is exposed through Web settings state and local diagnostics.
#ifdef _DEBUG
    std::wstring dbg = L"MouseFx: GPU route fallback detected. reason=";
    dbg += Utf8ToWString(reason);
    dbg += L"\n";
    OutputDebugStringW(dbg.c_str());
#else
    (void)reason;
#endif
}

void AppController::WriteGpuRouteStatusSnapshot(
    EffectCategory category,
    const std::string& requestedType,
    const std::string& effectiveType,
    const std::string& reason) const {
    if (category != EffectCategory::Hold) {
        return;
    }
    const std::wstring diagDir = ResolveLocalDiagDirectory();
    if (diagDir.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(diagDir, ec);
    if (ec) return;
    const std::filesystem::path file = std::filesystem::path(diagDir) / L"gpu_route_status_auto.json";
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;
    const std::string requestedNormalized = hold_route::NormalizeHoldEffectTypeAlias(requestedType);
    std::ostringstream ss;
    ss << "{"
       << "\"category\":\"hold\","
       << "\"requested\":\"" << requestedType << "\","
       << "\"requested_normalized\":\"" << requestedNormalized << "\","
       << "\"effective\":\"" << effectiveType << "\","
       << "\"fallback_applied\":" << (requestedNormalized == effectiveType ? "false" : "true") << ","
       << "\"reason\":\"" << reason << "\""
       << "}";
    out << ss.str();
}

void AppController::SetActiveEffectType(EffectCategory category, const std::string& type) {
    if (auto* slot = MutableActiveTypeForCategory(category); slot != nullptr) {
        *slot = type;
    }
}

void AppController::OnDispatchActivity(uint32_t msg, uintptr_t wParam) {
    const bool isMouseInputMsg =
        (msg == WM_MFX_CLICK || msg == WM_MFX_MOVE || msg == WM_MFX_SCROLL ||
         msg == WM_MFX_BUTTON_DOWN || msg == WM_MFX_BUTTON_UP || msg == WM_MFX_KEY);
    const bool isStateTimerMsg =
        (msg == WM_TIMER && (wParam == kHoverTimerId || wParam == kHoldTimerId));
    if (isMouseInputMsg || isStateTimerMsg) {
        UpdateVmSuppressionState();
    }

    if (!isMouseInputMsg) {
        return;
    }

    lastInputTime_ = CurrentTickMs();
    if (!hovering_) {
        return;
    }

    bool hoverEndRenderedByWasm = false;
    bool hoverWasmRouteActive = false;
    if (wasmEffectHost_ && wasmEffectHost_->Enabled() && wasmEffectHost_->IsPluginLoaded()) {
        hoverWasmRouteActive = true;
        ScreenPoint pt{};
        if (!QueryCursorScreenPoint(&pt)) {
            pt = ScreenPoint{};
        }
        wasm::EventInvokeInput invoke{};
        invoke.kind = wasm::EventKind::HoverEnd;
        invoke.x = pt.x;
        invoke.y = pt.y;
        invoke.eventTickMs = CurrentTickMs();
        std::vector<uint8_t> commandBuffer;
        const bool wasmOk = wasmEffectHost_->InvokeEvent(invoke, &commandBuffer);
        wasm::CommandExecutionResult execResult{};
        if (wasmOk && !commandBuffer.empty()) {
            const std::wstring manifestPath = wasmEffectHost_->Diagnostics().activeManifestPath;
            execResult = wasm::WasmClickCommandExecutor::Execute(
                commandBuffer.data(),
                commandBuffer.size(),
                config_,
                manifestPath);
            hoverEndRenderedByWasm = execResult.renderedAny;
        }
        wasmEffectHost_->RecordRenderExecution(
            hoverEndRenderedByWasm,
            execResult.executedTextCommands,
            execResult.executedImageCommands,
            execResult.droppedCommands,
            execResult.lastError);
    }

    hovering_ = false;
    if (!hoverWasmRouteActive || !hoverEndRenderedByWasm) {
        if (auto* effect = GetEffect(EffectCategory::Hover)) {
            effect->OnHoverEnd();
        }
    }
}

bool AppController::ConsumeIgnoreNextClick() {
    if (!ignoreNextClick_) {
        return false;
    }
    ignoreNextClick_ = false;
    return true;
}

void AppController::OnGlobalKey(const KeyEvent& ev) {
    const bool captureActiveBefore = shortcutCaptureSession_.IsActive();
    shortcutCaptureSession_.OnKeyDown(ev);
    const bool captureActiveAfter = shortcutCaptureSession_.IsActive();
    hook_->SetKeyboardCaptureExclusive(captureActiveAfter);
    if (captureActiveBefore || captureActiveAfter) {
        return;
    }
    inputIndicatorOverlay_->OnKey(ev);
}

std::string AppController::StartShortcutCaptureSession(uint64_t timeoutMs) {
    const std::string sessionId = shortcutCaptureSession_.Start(timeoutMs);
    hook_->SetKeyboardCaptureExclusive(shortcutCaptureSession_.IsActive());
    return sessionId;
}

void AppController::StopShortcutCaptureSession(const std::string& sessionId) {
    shortcutCaptureSession_.Stop(sessionId);
    hook_->SetKeyboardCaptureExclusive(shortcutCaptureSession_.IsActive());
}

ShortcutCaptureSession::PollResult AppController::PollShortcutCaptureSession(const std::string& sessionId) {
    ShortcutCaptureSession::PollResult result = shortcutCaptureSession_.Poll(sessionId);
    hook_->SetKeyboardCaptureExclusive(shortcutCaptureSession_.IsActive());
    return result;
}

bool AppController::ConsumeLatestMove(ScreenPoint* outPt) {
    if (!outPt) {
        return false;
    }
    return hook_->ConsumeLatestMove(*outPt);
}

uint64_t AppController::CurrentTickMs() const {
    if (!monotonicClockService_) {
        return 0;
    }
    return monotonicClockService_->NowMs();
}

uint32_t AppController::CurrentHoldDurationMs() const {
    if (!holdButtonDown_ || holdDownTick_ == 0) {
        return 0;
    }

    const uint64_t now = CurrentTickMs();
    const uint64_t delta = (now >= holdDownTick_) ? (now - holdDownTick_) : 0;
    return static_cast<uint32_t>(std::min<uint64_t>(delta, 0xFFFFFFFFu));
}

void AppController::BeginHoldTracking(const ScreenPoint& pt, int button) {
    holdButtonDown_ = true;
    holdDownTick_ = CurrentTickMs();
    pendingHold_.pt = pt;
    pendingHold_.button = button;
    pendingHold_.active = true;
    ignoreNextClick_ = false;
}

void AppController::EndHoldTracking() {
    holdButtonDown_ = false;
    holdDownTick_ = 0;
}

void AppController::ArmHoldTimer() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->SetTimer(kHoldTimerId, kHoldDelayMs);
    }
}

void AppController::DisarmHoldTimer() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoldTimerId);
    }
}

void AppController::ClearPendingHold() {
    pendingHold_.active = false;
}

void AppController::CancelPendingHold() {
    if (!pendingHold_.active) {
        return;
    }
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoldTimerId);
    }
    pendingHold_.active = false;
}

bool AppController::ConsumePendingHold(ScreenPoint* outPt, int* outButton) {
    if (!pendingHold_.active) {
        return false;
    }
    if (outPt) {
        *outPt = pendingHold_.pt;
    }
    if (outButton) {
        *outButton = pendingHold_.button;
    }
    pendingHold_.active = false;
    return true;
}

void AppController::MarkIgnoreNextClick() {
    ignoreNextClick_ = true;
}

bool AppController::TryEnterHover(ScreenPoint* outPt) {
    if (hovering_) {
        return false;
    }

    const uint64_t elapsed = CurrentTickMs() - lastInputTime_;
    if (elapsed < kHoverThresholdMs) {
        return false;
    }

    hovering_ = true;
    if (outPt) {
        if (!QueryCursorScreenPoint(outPt)) {
            outPt->x = 0;
            outPt->y = 0;
        }
    }
    return true;
}

bool AppController::QueryCursorScreenPoint(ScreenPoint* outPt) const {
    if (!outPt || !cursorPositionService_) {
        return false;
    }
    return cursorPositionService_->TryGetCursorScreenPoint(outPt);
}

std::string AppController::CurrentForegroundProcessBaseName() {
    if (!foregroundProcessService_) {
        return {};
    }
    return foregroundProcessService_->CurrentProcessBaseName();
}

void AppController::KillDispatchTimer(uintptr_t timerId) {
    if (!dispatchMessageHost_ || !dispatchMessageHost_->IsCreated()) {
        return;
    }
    dispatchMessageHost_->KillTimer(timerId);
}

#ifdef _DEBUG
void AppController::LogDebugClick(const ClickEvent& ev) {
    if (debugClickCount_ >= 5) {
        return;
    }
    debugClickCount_++;
    wchar_t buf[256]{};
    wsprintfW(buf, L"MouseFx: click received (%u) pt=(%ld,%ld) button=%u\n",
        debugClickCount_, ev.pt.x, ev.pt.y, static_cast<unsigned>(ev.button));
    OutputDebugStringW(buf);
}
#endif

bool AppController::Start() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) return true;
    diag_ = {};

    // Load config from the best available directory (AppData preferred)
    configDir_ = ResolveConfigDirectory();
    config_ = EffectConfig::Load(configDir_);
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(config_.holdPresenterBackend);
    InitializeWasmHost();
    inputIndicatorOverlay_->Initialize();
    inputIndicatorOverlay_->UpdateConfig(config_.inputIndicator);
    inputAutomationEngine_.UpdateConfig(config_.automation);

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

    if (NormalizeActiveEffectTypes()) {
        PersistConfig();
    }

    lastInputTime_ = CurrentTickMs();
    dispatchMessageHost_->SetTimer(kHoverTimerId, 100);

    diag_.stage = StartStage::GlobalHook;
    if (!hook_->Start(dispatchMessageHost_ ? dispatchMessageHost_->NativeHandle() : 0)) {
        const uint32_t hookError = hook_->LastError();
#ifdef _DEBUG
        wchar_t buf[256]{};
        wsprintfW(buf, L"MouseFx: global hook start failed. GetLastError=%lu\n", static_cast<unsigned long>(hookError));
        OutputDebugStringW(buf);
#endif
        diag_.error = static_cast<uint32_t>(hookError);
        Stop();
        return false;
    }

    return true;
}

void AppController::Stop() {
    ShutdownWasmHost();
    hook_->SetKeyboardCaptureExclusive(false);
    hook_->Stop();
    inputIndicatorOverlay_->Shutdown();
    inputAutomationEngine_.Reset();
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

// (Moved to top)
// Hold renderers are included in HoldEffect.cpp, but safe to include here too if needed, 
// though generally we rely on the creation site.
// Actually, AppController creates the Effects, but Effects create the Renderers (mostly).
// Except simpler effects might fallback?

std::unique_ptr<IMouseEffect> AppController::CreateEffect(EffectCategory category, const std::string& type) {
    return EffectFactory::Create(category, type, config_);
}

const std::string* AppController::ActiveTypeForCategory(EffectCategory category) const {
    for (const auto& descriptor : kActiveCategoryDescriptors) {
        if (descriptor.category != category) {
            continue;
        }
        return &(config_.active.*(descriptor.slot));
    }
    return nullptr;
}

std::string* AppController::MutableActiveTypeForCategory(EffectCategory category) {
    for (const auto& descriptor : kActiveCategoryDescriptors) {
        if (descriptor.category != category) {
            continue;
        }
        return &(config_.active.*(descriptor.slot));
    }
    return nullptr;
}

bool AppController::IsActiveEffectEnabled(EffectCategory category) const {
    const std::string* activeType = ActiveTypeForCategory(category);
    return (activeType != nullptr && !activeType->empty() && *activeType != "none");
}

void AppController::ReapplyActiveEffect(EffectCategory category) {
    if (category == EffectCategory::Click) {
        SetEffect(category, ResolveConfiguredClickType());
        return;
    }
    const std::string* activeType = ActiveTypeForCategory(category);
    if (activeType == nullptr) {
        return;
    }
    SetEffect(category, *activeType);
}

std::string AppController::ResolveConfiguredClickType() const {
    if (!config_.active.click.empty()) {
        return config_.active.click;
    }
    if (!config_.defaultEffect.empty()) {
        return config_.defaultEffect;
    }
    return "ripple";
}

void AppController::ApplyConfiguredEffects() {
    for (const auto& descriptor : kActiveCategoryDescriptors) {
        const std::string requestedType =
            (descriptor.category == EffectCategory::Click)
                ? ResolveConfiguredClickType()
                : (config_.active.*(descriptor.slot));
        SetEffect(descriptor.category, requestedType);
    }
}

bool AppController::NormalizeActiveEffectTypes() {
    bool normalizedChanged = false;
    for (const auto& descriptor : kActiveCategoryDescriptors) {
        std::string& slot = config_.active.*(descriptor.slot);
        std::string reason;
        const std::string effective = ResolveRuntimeEffectType(descriptor.category, slot, &reason);
        if (slot == effective) {
            continue;
        }
        slot = effective;
        normalizedChanged = true;
    }
    return normalizedChanged;
}

void AppController::SetEffect(EffectCategory category, const std::string& type) {
    size_t idx = static_cast<size_t>(category);
    if (idx >= kCategoryCount) return;

    std::string fallbackReason;
    const std::string requestedNormalized =
        (category == EffectCategory::Hold) ? hold_route::NormalizeHoldEffectTypeAlias(type) : type;
    const std::string effectiveType = ResolveRuntimeEffectType(category, type, &fallbackReason);
    if (!fallbackReason.empty() && effectiveType != requestedNormalized) {
        NotifyGpuFallbackIfNeeded(fallbackReason);
    }
    WriteGpuRouteStatusSnapshot(category, type, effectiveType, fallbackReason);

    // Shutdown existing effect for this category
    if (effects_[idx]) {
        effects_[idx]->Shutdown();
        effects_[idx].reset();
    }

    // Create and initialize new effect
    effects_[idx] = CreateEffect(category, effectiveType);
    if (effects_[idx] && !vmEffectsSuppressed_) {
        effects_[idx]->Initialize();
    }

#ifdef _DEBUG
    wchar_t buf[256]{};
    wsprintfW(buf, L"MouseFx: SetEffect category=%hs type=%hs\n", 
              CategoryToString(category), effectiveType.c_str());
    OutputDebugStringW(buf);
#endif
}

void AppController::ClearEffect(EffectCategory category) {
    SetEffect(category, "none");
}

void AppController::SetTheme(const std::string& theme) {
    if (theme.empty()) return;
    config_.theme = theme;
    // Re-create themed effects to pick up new palette.
    for (const auto& descriptor : kActiveCategoryDescriptors) {
        if (!descriptor.themeSensitive) {
            continue;
        }
        ReapplyActiveEffect(descriptor.category);
    }
    PersistConfig();
}

void AppController::SetHoldFollowMode(const std::string& mode) {
    const std::string normalized = NormalizeHoldFollowMode(mode);
    if (config_.holdFollowMode == normalized) return;
    config_.holdFollowMode = normalized;
    PersistConfig();
    if (IsActiveEffectEnabled(EffectCategory::Hold)) {
        ReapplyActiveEffect(EffectCategory::Hold);
    }
}

void AppController::SetHoldPresenterBackend(const std::string& backend) {
    const std::string normalized = config_internal::NormalizeHoldPresenterBackend(backend);
    if (config_.holdPresenterBackend == normalized) {
        return;
    }
    config_.holdPresenterBackend = normalized;
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(config_.holdPresenterBackend);
    PersistConfig();
    if (IsActiveEffectEnabled(EffectCategory::Hold)) {
        ReapplyActiveEffect(EffectCategory::Hold);
    }
}

IMouseEffect* AppController::GetEffect(EffectCategory category) const {
    size_t idx = static_cast<size_t>(category);
    if (idx >= kCategoryCount) return nullptr;
    return effects_[idx].get();
}

void AppController::HandleCommand(const std::string& jsonCmd) {
    if (!dispatchMessageHost_ || !dispatchMessageHost_->IsCreated()) return;

    // Thread Safety: Marshal to UI thread if we are on a background thread.
    if (!dispatchMessageHost_->IsOwnerThread()) {
        dispatchMessageHost_->SendSync(WM_MFX_EXEC_CMD, 0, reinterpret_cast<intptr_t>(&jsonCmd));
        return;
    }

    commandHandler_->Handle(jsonCmd);
}

bool AppController::CreateDispatchWindow() {
    if (!dispatchMessageHost_) {
        diag_.error = static_cast<uint32_t>(ERROR_INVALID_HANDLE);
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

void AppController::UpdateVmSuppressionState() {
    if (!vmForegroundDetector_) {
        return;
    }
    const uint64_t now = CurrentTickMs();
    const bool suppress = vmForegroundDetector_->ShouldSuppress(now);
    if (suppress == vmEffectsSuppressed_) return;
    ApplyVmSuppression(suppress);
}

void AppController::ApplyVmSuppression(bool suppressed) {
    if (suppressed) {
        SuspendEffectsForVm();
    } else {
        ResumeEffectsAfterVm();
    }
    vmEffectsSuppressed_ = suppressed;
}

void AppController::SuspendEffectsForVm() {
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoldTimerId);
    }
    pendingHold_.active = false;
    ignoreNextClick_ = false;
    holdButtonDown_ = false;
    holdDownTick_ = 0;
    hovering_ = false;
    inputIndicatorOverlay_->Hide();
    inputAutomationEngine_.Reset();

    for (auto& effect : effects_) {
        if (effect) effect->Shutdown();
    }
}

void AppController::ResumeEffectsAfterVm() {
    for (auto& effect : effects_) {
        if (effect) effect->Initialize();
    }
}

intptr_t AppController::OnDispatchMessage(
    uintptr_t sourceHandle,
    uint32_t msg,
    uintptr_t wParam,
    intptr_t lParam) {
    if (!dispatchRouter_ || !dispatchMessageCodec_) {
        return 0;
    }

    const DispatchMessage decoded = dispatchMessageCodec_->Decode(sourceHandle, msg, wParam, lParam);
    bool handled = false;
    const intptr_t routeResult = dispatchRouter_->Route(decoded, &handled);
    if (handled) {
        return routeResult;
    }
    return dispatchMessageCodec_->DefaultResult(sourceHandle, msg, wParam, lParam);
}


} // namespace mousefx


