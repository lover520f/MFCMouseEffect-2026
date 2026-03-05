#include "pch.h"

#include "AppController.h"

namespace mousefx {

uint64_t AppController::VmForegroundSuppressionCheckIntervalMs() const {
    if (!foregroundSuppressionService_) {
        return 0;
    }
    return foregroundSuppressionService_->CheckIntervalMsForDiagnostics();
}

void AppController::UpdateVmSuppressionState() {
    if (!foregroundSuppressionService_) {
        return;
    }
    const uint64_t now = CurrentTickMs();
    const bool suppress = foregroundSuppressionService_->ShouldSuppress(now);
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
        dispatchMessageHost_->KillTimer(kHoldUpdateTimerId);
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
        if (effect) effect->Shutdown();
    }
}

void AppController::ResumeEffectsAfterVm() {
    for (auto& effect : effects_) {
        if (effect) effect->Initialize();
    }
}

} // namespace mousefx
