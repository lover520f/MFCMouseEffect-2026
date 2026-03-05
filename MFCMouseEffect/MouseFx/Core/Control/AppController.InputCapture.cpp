#include "pch.h"
#include "AppController.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/System/MacosInputPermissionState.h"
#endif

namespace mousefx {
namespace {

constexpr uint32_t kPosixPermissionDeniedError = 13;

} // namespace

AppController::InputCaptureRuntimeStatus AppController::InputCaptureStatus() const {
    const bool active = inputCaptureActive_.load(std::memory_order_acquire);
    const uint32_t error = inputCaptureError_.load(std::memory_order_acquire);
    InputCaptureRuntimeStatus status{};
    status.active = active;
    status.error = error;
    status.reason = ClassifyInputCaptureFailure(active, error);
    return status;
}

bool AppController::EffectsSuspendedByInputCapture() const {
    return effectsSuspendedByInputCapture_.load(std::memory_order_acquire);
}

void AppController::SetInputCaptureStatusCallback(
    std::function<void(const InputCaptureRuntimeStatus&)> callback) {
    std::lock_guard<std::mutex> lock(inputCaptureStatusCallbackMutex_);
    inputCaptureStatusCallback_ = std::move(callback);
}

void AppController::NotifyInputCaptureStatusChanged() {
    std::function<void(const InputCaptureRuntimeStatus&)> callback;
    {
        std::lock_guard<std::mutex> lock(inputCaptureStatusCallbackMutex_);
        callback = inputCaptureStatusCallback_;
    }
    if (!callback) {
        return;
    }
    callback(InputCaptureStatus());
}

AppController::InputCaptureFailureReason AppController::ClassifyInputCaptureFailure(
    bool active,
    uint32_t error) {
    if (active) {
        return InputCaptureFailureReason::None;
    }
#if MFX_PLATFORM_MACOS
    if (error == kPosixPermissionDeniedError) {
        return InputCaptureFailureReason::PermissionDenied;
    }
#endif
#if MFX_PLATFORM_LINUX
    if (error == 0) {
        return InputCaptureFailureReason::Unsupported;
    }
#endif
    return InputCaptureFailureReason::StartFailed;
}

void AppController::RefreshInputCaptureRuntimeState() {
#if MFX_PLATFORM_MACOS
    if (!hook_) {
        return;
    }

    uint32_t hookError = hook_->LastError();
    const bool captureActive = inputCaptureActive_.load(std::memory_order_acquire);
    if (!captureActive && hookError != 0 && dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        const uint64_t nowTickMs = CurrentTickMs();
        if (nowTickMs == 0 || nowTickMs >= lastInputCaptureRestartAttemptTickMs_ + kInputCaptureRestartRetryMs) {
            lastInputCaptureRestartAttemptTickMs_ = nowTickMs;
            (void)hook_->Start(dispatchMessageHost_.get());
            hookError = hook_->LastError();
        }
    }
    if (hookError == 0) {
        inputCaptureTransientErrorCode_ = 0;
        inputCaptureTransientErrorSinceTickMs_ = 0;
        if (captureActive) {
            return;
        }

        const uint32_t previousError = inputCaptureError_.load(std::memory_order_acquire);
        if (previousError == 0) {
            return;
        }

        inputCaptureActive_.store(true, std::memory_order_release);
        inputCaptureError_.store(0, std::memory_order_release);
        effectsSuspendedByInputCapture_.store(false, std::memory_order_release);
        inputCaptureTransientErrorCode_ = 0;
        inputCaptureTransientErrorSinceTickMs_ = 0;
        hovering_ = false;
        pendingHold_.active = false;
        holdButtonDown_ = false;
        holdTrackingButton_ = 0;
        holdDownTick_ = 0;
        lastInputCaptureRestartAttemptTickMs_ = 0;
        ignoreNextClick_ = false;
        if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
            dispatchMessageHost_->SetTimer(kHoverTimerId, 100);
        }
        hook_->SetKeyboardCaptureExclusive(shortcutCaptureSession_.IsActive());
        inputAutomationEngine_.Reset();
        if (!vmEffectsSuppressed_) {
            for (auto& effect : effects_) {
                if (effect) {
                    effect->Initialize();
                }
            }
        }
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: input capture recovered at runtime.\n");
#endif
        NotifyInputCaptureStatusChanged();
        return;
    }

    if (!captureActive) {
        if (inputCaptureError_.load(std::memory_order_acquire) == 0) {
            inputCaptureError_.store(hookError, std::memory_order_release);
            NotifyInputCaptureStatusChanged();
        }
        return;
    }

    if (hookError == kPosixPermissionDeniedError) {
        const std::string simulationFilePath = macos_input_permission::ReadPermissionSimulationFilePath();
        const bool runtimeTrusted =
            macos_input_permission::IsRuntimeInputTrusted(simulationFilePath);
        if (!runtimeTrusted) {
            EnterInputCaptureDegradedMode(hookError);
            return;
        }
        // Guard against false permission-denied blips while runtime permission
        // is still trusted (for example transient event-tap state churn).
        inputCaptureTransientErrorCode_ = 0;
        inputCaptureTransientErrorSinceTickMs_ = 0;
        return;
    }

    const uint64_t nowTickMs = CurrentTickMs();
    if (inputCaptureTransientErrorCode_ != hookError) {
        inputCaptureTransientErrorCode_ = hookError;
        inputCaptureTransientErrorSinceTickMs_ = nowTickMs;
        return;
    }
    if (inputCaptureTransientErrorSinceTickMs_ == 0) {
        inputCaptureTransientErrorSinceTickMs_ = nowTickMs;
        return;
    }
    if (nowTickMs == 0 ||
        nowTickMs < inputCaptureTransientErrorSinceTickMs_ + kInputCaptureTransientErrorGraceMs) {
        return;
    }

    EnterInputCaptureDegradedMode(hookError);
#endif
}

void AppController::EnterInputCaptureDegradedMode(uint32_t error) {
    inputCaptureActive_.store(false, std::memory_order_release);
    inputCaptureError_.store(error, std::memory_order_release);
    effectsSuspendedByInputCapture_.store(true, std::memory_order_release);
    inputCaptureTransientErrorCode_ = 0;
    inputCaptureTransientErrorSinceTickMs_ = 0;
    NotifyInputCaptureStatusChanged();
    lastInputCaptureRestartAttemptTickMs_ = 0;
    if (dispatchMessageHost_ && dispatchMessageHost_->IsCreated()) {
        dispatchMessageHost_->KillTimer(kHoverTimerId);
        dispatchMessageHost_->KillTimer(kHoldTimerId);
        dispatchMessageHost_->KillTimer(kHoldUpdateTimerId);
    }
    if (hook_) {
        hook_->SetKeyboardCaptureExclusive(false);
    }
    pendingHold_.active = false;
    ignoreNextClick_ = false;
    holdButtonDown_ = false;
    holdTrackingButton_ = 0;
    holdDownTick_ = 0;
    hovering_ = false;
    inputIndicatorOverlay_->Hide();
    inputAutomationEngine_.Reset();
    for (auto& effect : effects_) {
        if (effect) {
            effect->Shutdown();
        }
    }
#ifdef _DEBUG
    wchar_t buf[256]{};
    wsprintfW(buf, L"MouseFx: input capture degraded at runtime. error=%lu\n",
              static_cast<unsigned long>(error));
    OutputDebugStringW(buf);
#endif
}

} // namespace mousefx
