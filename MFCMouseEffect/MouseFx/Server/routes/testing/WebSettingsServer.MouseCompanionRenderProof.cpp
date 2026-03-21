#include "pch.h"

#include "MouseFx/Server/routes/testing/WebSettingsServer.MouseCompanionRenderProof.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace mousefx {
namespace {

AppController::MouseCompanionRuntimeStatus WaitForRendererRuntimeFrameAdvance(
    AppController* controller,
    const AppController::MouseCompanionRuntimeStatus& beforeStatus,
    uint32_t waitForFrameMs) {
    AppController::MouseCompanionRuntimeStatus latestStatus =
        controller ? controller->ReadMouseCompanionRuntimeStatus()
                   : AppController::MouseCompanionRuntimeStatus{};
    if (!controller || waitForFrameMs == 0 ||
        DidMouseCompanionRenderProofAdvanceFrame(beforeStatus, latestStatus)) {
        return latestStatus;
    }

    const uint32_t deadlineMs = std::min<uint32_t>(waitForFrameMs, 2000);
    constexpr uint32_t kPollIntervalMs = 8;
    uint32_t waitedMs = 0;
    while (waitedMs < deadlineMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
        waitedMs += kPollIntervalMs;
        latestStatus = controller->ReadMouseCompanionRuntimeStatus();
        if (DidMouseCompanionRenderProofAdvanceFrame(beforeStatus, latestStatus)) {
            break;
        }
    }
    return latestStatus;
}

} // namespace

bool DidMouseCompanionRenderProofAdvanceFrame(
    const AppController::MouseCompanionRuntimeStatus& before,
    const AppController::MouseCompanionRuntimeStatus& after) {
    return after.rendererRuntimeFrameCount > before.rendererRuntimeFrameCount ||
           after.rendererRuntimeLastRenderTickMs > before.rendererRuntimeLastRenderTickMs;
}

MouseCompanionRenderProofResult CaptureMouseCompanionRenderProof(
    AppController* controller,
    const AppController::MouseCompanionRuntimeStatus& beforeStatus,
    uint32_t waitForFrameMs,
    bool expectFrameAdvance) {
    MouseCompanionRenderProofResult result{};
    result.beforeStatus = beforeStatus;
    result.waitForFrameMs = std::min<uint32_t>(waitForFrameMs, 2000);
    result.expectFrameAdvance = expectFrameAdvance;
    result.afterStatus =
        WaitForRendererRuntimeFrameAdvance(controller, beforeStatus, result.waitForFrameMs);
    const bool frameAdvanced =
        DidMouseCompanionRenderProofAdvanceFrame(result.beforeStatus, result.afterStatus);
    result.expectationMet = !expectFrameAdvance || frameAdvanced;
    result.expectationStatus =
        !expectFrameAdvance ? "not_requested" : (frameAdvanced ? "frame_advanced" : "frame_not_advanced");
    return result;
}

} // namespace mousefx
