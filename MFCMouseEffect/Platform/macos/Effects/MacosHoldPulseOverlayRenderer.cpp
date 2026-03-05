#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <memory>
#include <mutex>

namespace mousefx::macos_hold_pulse {

namespace {

#if defined(__APPLE__)
std::mutex gUpdateMutex{};
bool gUpdateDrainScheduled = false;
bool gUpdatePending = false;
HoldEffectUpdateCommand gLatestUpdateCommand{};

struct StartHoldPulseContext final {
    HoldEffectStartCommand command{};
    std::string themeName{};
};

void StartHoldPulseOverlayCallback(void* opaque) {
    std::unique_ptr<StartHoldPulseContext> context(
        static_cast<StartHoldPulseContext*>(opaque));
    if (!context) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(gUpdateMutex);
        gUpdatePending = false;
        gUpdateDrainScheduled = false;
    }
    StartHoldPulseOverlayOnMain(context->command, context->themeName);
}

void DrainHoldPulseOverlayUpdatesCallback(void*) {
    HoldEffectUpdateCommand command{};
    {
        std::lock_guard<std::mutex> lock(gUpdateMutex);
        if (!gUpdatePending) {
            gUpdateDrainScheduled = false;
            return;
        }
        command = gLatestUpdateCommand;
        gUpdatePending = false;
    }
    UpdateHoldPulseOverlayOnMain(command);

    bool needScheduleNextDrain = false;
    {
        std::lock_guard<std::mutex> lock(gUpdateMutex);
        if (gUpdatePending) {
            needScheduleNextDrain = true;
        } else {
            gUpdateDrainScheduled = false;
        }
    }
    if (needScheduleNextDrain) {
        macos_overlay_support::RunOnMainThreadAsync(&DrainHoldPulseOverlayUpdatesCallback, nullptr);
    }
}

void StopHoldPulseOverlayCallback(void*) {
    {
        std::lock_guard<std::mutex> lock(gUpdateMutex);
        gUpdatePending = false;
        gUpdateDrainScheduled = false;
    }
    CloseHoldPulseOverlayOnMain();
}

struct ActiveHoldWindowCountContext final {
    size_t* count = nullptr;
};

void CaptureActiveHoldWindowCountCallback(void* opaque) {
    auto* context = static_cast<ActiveHoldWindowCountContext*>(opaque);
    if (context == nullptr || context->count == nullptr) {
        return;
    }
    *context->count = GetActiveHoldPulseWindowCountOnMain();
}
#endif

} // namespace

void StartHoldPulseOverlay(const HoldEffectStartCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    auto* context = new StartHoldPulseContext{
        command,
        themeName,
    };
    macos_overlay_support::RunOnMainThreadAsync(&StartHoldPulseOverlayCallback, context);
#endif
}

void UpdateHoldPulseOverlay(const HoldEffectUpdateCommand& command) {
#if !defined(__APPLE__)
    (void)command;
    return;
#else
    bool needScheduleDrain = false;
    {
        std::lock_guard<std::mutex> lock(gUpdateMutex);
        gLatestUpdateCommand = command;
        gUpdatePending = true;
        if (!gUpdateDrainScheduled) {
            gUpdateDrainScheduled = true;
            needScheduleDrain = true;
        }
    }
    if (needScheduleDrain) {
        macos_overlay_support::RunOnMainThreadAsync(&DrainHoldPulseOverlayUpdatesCallback, nullptr);
    }
#endif
}

void StopHoldPulseOverlay() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(&StopHoldPulseOverlayCallback, nullptr);
#endif
}

size_t GetActiveHoldPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    size_t count = 0;
    ActiveHoldWindowCountContext context{&count};
    macos_overlay_support::RunOnMainThreadSync(&CaptureActiveHoldWindowCountCallback, &context);
    return count;
#endif
}

} // namespace mousefx::macos_hold_pulse
