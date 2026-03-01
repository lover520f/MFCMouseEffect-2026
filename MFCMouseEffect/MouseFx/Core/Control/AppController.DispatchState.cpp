#include "pch.h"

#include "AppController.h"

#include <algorithm>

namespace mousefx {

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
        if (QueryCursorScreenPoint(outPt)) {
            RememberLastPointerPoint(*outPt);
        } else if (!TryGetLastPointerPoint(outPt)) {
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

void AppController::RememberLastPointerPoint(const ScreenPoint& pt) {
    lastPointerPoint_ = pt;
    hasLastPointerPoint_ = true;
}

bool AppController::TryGetLastPointerPoint(ScreenPoint* outPt) const {
    if (!outPt || !hasLastPointerPoint_) {
        return false;
    }
    *outPt = lastPointerPoint_;
    return true;
}

std::string AppController::CurrentForegroundProcessBaseName() {
    if (!foregroundProcessService_) {
        return {};
    }
    return foregroundProcessService_->CurrentProcessBaseName();
}

bool AppController::InjectShortcutForTest(const std::string& chordText) {
    if (!keyboardInjector_) {
        return false;
    }
    return keyboardInjector_->SendChord(chordText);
}

void AppController::KillDispatchTimer(uintptr_t timerId) {
    if (!dispatchMessageHost_ || !dispatchMessageHost_->IsCreated()) {
        return;
    }
    dispatchMessageHost_->KillTimer(timerId);
}

} // namespace mousefx
