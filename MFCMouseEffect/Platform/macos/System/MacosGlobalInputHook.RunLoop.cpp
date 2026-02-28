#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"
#include <string>

#include "Platform/macos/System/MacosInputPermissionState.h"

namespace mousefx {

void MacosGlobalInputHook::RunEventTapLoop() {
#if !defined(__APPLE__)
    NotifyInitResult(false, kErrorTapCreateFailed);
    return;
#else
    const std::string simulationFilePath = macos_input_permission::ReadPermissionSimulationFilePath();
    const bool trusted = macos_input_permission::IsRuntimeInputTrusted(simulationFilePath);
    if (RunPermissionSimulationLoop(simulationFilePath, trusted)) {
        return;
    }

    const CGEventMask mask = ComputeEventTapMask();

    CFMachPortRef tap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        mask,
        MacosGlobalInputHook::EventTapCallback,
        this);
    if (!tap) {
        NotifyInitResult(false, trusted ? kErrorTapCreateFailed : kErrorPermissionDenied);
        return;
    }

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
    if (!source) {
        CFRelease(tap);
        NotifyInitResult(false, kErrorSourceCreateFailed);
        return;
    }

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    CFRunLoopTimerRef permissionProbeTimer = nullptr;
    CFRunLoopTimerContext timerContext{};
    timerContext.info = this;
    permissionProbeTimer = CFRunLoopTimerCreate(
        kCFAllocatorDefault,
        CFAbsoluteTimeGetCurrent() + kPermissionProbeIntervalSeconds,
        kPermissionProbeIntervalSeconds,
        0,
        0,
        MacosGlobalInputHook::PermissionProbeTimerCallback,
        &timerContext);
    {
        std::lock_guard<std::mutex> lock(runLoopMutex_);
        runLoopRef_ = runLoop;
        tapRef_ = tap;
        sourceRef_ = source;
    }

    CFRunLoopAddSource(runLoop, source, kCFRunLoopCommonModes);
    if (permissionProbeTimer != nullptr) {
        CFRunLoopAddTimer(runLoop, permissionProbeTimer, kCFRunLoopCommonModes);
    }
    CGEventTapEnable(tap, true);
    NotifyInitResult(true, kErrorSuccess);

    CFRunLoopRun();

    if (permissionProbeTimer != nullptr) {
        CFRunLoopRemoveTimer(runLoop, permissionProbeTimer, kCFRunLoopCommonModes);
    }
    CFRunLoopRemoveSource(runLoop, source, kCFRunLoopCommonModes);
    {
        std::lock_guard<std::mutex> lock(runLoopMutex_);
        runLoopRef_ = nullptr;
        tapRef_ = nullptr;
        sourceRef_ = nullptr;
    }
    if (permissionProbeTimer != nullptr) {
        CFRelease(permissionProbeTimer);
    }
    CFRelease(source);
    CFRelease(tap);
#endif
}

} // namespace mousefx
