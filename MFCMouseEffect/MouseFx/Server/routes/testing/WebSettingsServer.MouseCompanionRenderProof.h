#pragma once

#include <cstdint>
#include <string>

#include "MouseFx/Core/Control/AppController.h"

namespace mousefx {

struct MouseCompanionRenderProofResult final {
    AppController::MouseCompanionRuntimeStatus beforeStatus{};
    AppController::MouseCompanionRuntimeStatus afterStatus{};
    uint32_t waitForFrameMs{0};
    bool expectFrameAdvance{false};
    bool expectationMet{true};
    std::string expectationStatus{"not_requested"};
};

MouseCompanionRenderProofResult CaptureMouseCompanionRenderProof(
    AppController* controller,
    const AppController::MouseCompanionRuntimeStatus& beforeStatus,
    uint32_t waitForFrameMs,
    bool expectFrameAdvance);

bool DidMouseCompanionRenderProofAdvanceFrame(
    const AppController::MouseCompanionRuntimeStatus& before,
    const AppController::MouseCompanionRuntimeStatus& after);

} // namespace mousefx
